#pragma once
#include<iostream>
#include<vector>
#include<regex>
#include<vector>
#include <string>
#include <algorithm>
#include<chrono>
using namespace std;


enum Attributes {
	FDir = 0B1,
	FLabel = 0B10,
	FWrite = 0B100,
	FExec = 0B1000,
	FRead = 0B10000
};

const int MassiveSize = 100;


class Buffer {
protected:
	static const int size = MassiveSize;
	int* _val = new int[size];
	int nulv;
public:
	int GetSize() { return size; }
	Buffer(int v = 0) {
		for (int i = 0; i < size; i++) {
			_val[i] = v;
			nulv = v;
		}
	}

	bool IsFree(int id) { if (id < size) { return _val[id] == nulv; }return false; }
	int GetFirstFree(int len = 1, bool FromEnd = false);
	void Write(int id, int len, int val = 1) {
		for (int i = 0; i < len; i++)
		{
			_val[id + i] = val;
		}
	};
	void Clear(int id, int len) { for (int i = 0; i < len; i++) { _val[id + i] = nulv; } };
	void DampOnScreen() { std::cout << "["; for (int i = 0; i < size; i++) { std::cout << _val[i]; }std::cout << "]\n"; }
	bool Free(int id) { return _val[id] == nulv; }
	int GetVal(int id) { return _val[id]; }

};
class PathHolder {
protected:
	string full;
	int curpos = 0;
	int cursize = 0;
	int depth = 0;
	int size = 0;
public:
	PathHolder(string p) :full(p) {
		cursize = full.find("/", curpos + cursize);
		size = std::count(full.begin(), full.end(), '/') + 1;
	};
	string GetCurrent() {
		return full.substr(curpos, cursize);
	}
	bool NextIsLast() { return (size - depth) < 2; }
	void Next() {
		curpos = curpos + cursize + 1;
		cursize = full.find("/", curpos) - curpos;
		depth++;
		if (cursize < 0) {
			cursize = full.size() - curpos;
		}
	}
};
class AttributeHolder {
protected:
	int val = 0;
public:
	AttributeHolder(int v = 0) :val(v) {};
	void SetCode(int code) { val = code; }
	void SetFlag(Attributes a) {
		val |= a;
	}
	void UnSetFlag(Attributes a) {
		val &= ~a;
	}
	bool CheckFlag(Attributes a) {
		return (val & a) == a;
	}
	int GetFlag() { return val; }
};

const int FILE_NAME_LENGTH_LIMIT = 10;
const int MAX_PATH_SIZE = 10;

class RamDisplay {
private:
	int size;
	int pos;
public:
	void Push(int sz);
	void Clear();
};


class IFileRealization {
protected:
	int size = 0;
	int linkCount = 1;
	RamDisplay rd;
public:
	virtual int GetSize() { return size; }
	virtual void Fill(int size) { cout << "sending Data to HDD\n"; }
	virtual void Erase() { cout << "clearing Data from HDD\n"; }
	virtual void Open();
	virtual void Close();

	virtual void Link() { linkCount++; }
	virtual void Unlink() { linkCount--; if (linkCount < 1) { Close(); Erase(); delete this; } }
	virtual int GetFirstDiskClaster(bool& ReturnedRealValue) { ReturnedRealValue = false; return -1; }
};

class FSNote {
public:
	virtual void Open() { cout << "This operation can only be used with Files, not Directories\n"; }; //file //inReal
	virtual void Close() { cout << "This operation can only be used with Files, not Directories\n"; }; //file //inReal
	virtual string GetName() { return ""; cout << "NullName\n"; };													//both
	virtual void SetName(string) { cout << "Realisation not found\n"; };								//both
	virtual void Fill(int size) { cout << "This operation can only be used with Files, not Directories\n"; };//file //inReal
	virtual void Erase() { cout << "this operation only used for clearing HDD or RAM and not allowed for Directories\n"; }//file //inReal
	virtual void Delete() { cout << "Note Destroyed with its value object\n"; };//both 
	virtual void SetCode(int code) { cout << "Set all attributes via single code\n"; }//both
	virtual void SetFlag(Attributes fl) { cout << "Attribute Cant be set on this type of object\n"; }//both
	virtual void UnSetFlag(Attributes fl) { cout << "Attribute Cant be set on this type of object\n"; }//both
	virtual bool CheckFlag(Attributes fl) { return 0; }//both
	virtual int Link() { cout << "This operation is only allowed for files and used for increasing copies count\n"; return 0; }//files //inReal
	virtual vector<FSNote*> GetChildren() { cout << "This operation returns zero vector for files\n"; vector<FSNote*> k; return k; }//dirs
	virtual int AddChild(FSNote*) { cout << "this operation is undefied\n"; return 0; }//dirs
	virtual void DeleteChild(string, int x = 0) { cout << "This operation is Undefied\n"; }//dirs
	virtual int GetFlagCode() { cout << "unsexpected usage, probably you got NullPointer\n"; return 0; }//both
	virtual FSNote* GetNoteFromName(string name, bool& Founded) { return nullptr; cout << "this operation only can be used with directories, for files it always sets Founded bit to false\n"; }//dirs
	virtual int GetSize() { cout << "Size of Directory is taken as 1\n"; return 1; } // files
	virtual IFileRealization* GetVal(bool& ValReturned) { cout << "this function only uses for files\n"; ValReturned = false; return nullptr; }
	virtual void SetVal(IFileRealization*) { cout << "this function only uses for files\n"; }
	virtual int GetFirstDiskClaster(bool& ReturnedRealValue) { ReturnedRealValue = false; return -1; }
	virtual FSNote* Copy() { cout << "returning copy of note\n"; return nullptr; }
};

class FSNodeFactory {
public:
	virtual FSNote* CreateFileNote() = 0;
	virtual FSNote* CreateDirNote() = 0;
	virtual FSNote* CreateLabel() = 0;
	virtual bool AllowDefragmentation() { return false; }
};

class NameRealisation {
public:
	virtual void SetName(string name) = 0;
	virtual string GetName() = 0;
};

class BruteNameContainer :public NameRealisation {
private:
	string name;
public:
	void SetName(string nm) { name = nm; }
	string GetName() { return name; }
};

class HeapNameContainer :public NameRealisation {
private:
	char* name;
	int curlen = 0;
public:
	void SetName(string nm) {
		delete name;
		curlen = nm.size() + 1;
		name = new char[curlen] {' '};
		strcpy_s(name, curlen, nm.c_str());

	}
	string GetName() {
		return string(name);
	}
};

class StandartDirNoteRealisation :public FSNote {
protected:
	virtual void AbstractFlag() = 0;
	NameRealisation* nme;
	AttributeHolder atts;
	vector<FSNote*> children;
public:
	void SetName(string nm) {
		nme->SetName(nm);
	}
	string GetName() {
		return nme->GetName();
	}
	void SetFlag(Attributes x) { atts.SetFlag(x); }
	void UnSetFlag(Attributes x) { atts.UnSetFlag(x); }
	bool CheckFlag(Attributes x) { return atts.CheckFlag(x); }
	void SetCode(int code) { atts.SetCode(code); }
	int GetFlagCode() { return atts.GetFlag(); }
	void Delete() {
		for (auto x : children) {
			x->Delete();
			delete x;
		}
		children.clear();
	}
	vector<FSNote*> GetChildren() { return children; }
	int AddChild(FSNote* toAdd);
	FSNote* GetNoteFromName(string name, bool& Founded);
	void DeleteChild(string name, int code = 0);
	virtual FSNote* Copy() { return nullptr; };
};

class FullnameDirNote :public StandartDirNoteRealisation {
protected:
	void AbstractFlag() { cout << "flag defused\n"; }
public:
	FullnameDirNote() { nme = new BruteNameContainer(); }
	FSNote* Copy() {
		FullnameDirNote* dir = new FullnameDirNote();
		dir->nme = new BruteNameContainer();
		dir->SetName(GetName());
		dir->atts = atts;
		for (auto x : children) {
			dir->children.push_back(x->Copy());
		}
		return (FSNote*)dir;
	};
};

class HeapNameDirNote :public StandartDirNoteRealisation {
protected:
	void AbstractFlag() { cout << "flag defused\n"; }
public:
	HeapNameDirNote() { nme = new HeapNameContainer(); }
};



class ContFileRealization :public IFileRealization {
protected:
	int pos = -1;
public:
	void Fill(int s);
	void Erase();
	int GetFirstDiskClaster(bool& ReturnedRealValue) { ReturnedRealValue = (pos != -1); return pos; }
	void Shift(int CountOfFree);
};

class Link1 {
public:
	Link1* Next = nullptr;
	int val = -1;
	void Delete() {
		if (Next != nullptr) {
			Next->Delete();
			delete Next;
		}
	}
};
class LinkLIstRealization :public IFileRealization {
protected:
	int linkCount = 1;
	Link1* First;
public:
	virtual void Fill(int sz);
	virtual void Erase();
};
class FATRealizaton :public IFileRealization {
protected:
	int startind = -2;
public:
	void Fill(int sz);
	void Erase();
};


class StandartFileNoteRealisation :public FSNote {
protected:
	virtual void AbstractFlag() = 0;
	NameRealisation* nme;
	AttributeHolder atts;
	IFileRealization* file;
public:
	void SetName(string nm) {
		nme->SetName(nm);
	}
	string GetName() {
		return nme->GetName();
	}
	void SetFlag(Attributes a) { atts.SetFlag(a); }
	void UnSetFlag(Attributes a) { atts.UnSetFlag(a); }
	bool CheckFlag(Attributes a) { return atts.CheckFlag(a); }
	void SetCode(int code) { atts.SetCode(code); }

	void Open() { file->Open(); }
	void Close() { file->Close(); }
	void Fill(int size) { file->Fill(size); }
	void Erase() { file->Erase(); }
	void Delete() { file->Unlink(); }

	int Link() { file->Link(); return 1; }
	int GetSize() { return file->GetSize(); }
	int GetFlagCode() { return atts.GetFlag(); }
	IFileRealization* GetVal(bool& ValReturned) { ValReturned = true; return file; }
	void SetVal(IFileRealization* f) {
		file->Unlink();
		file = f;
	}

};

class INodeFileRealization :public IFileRealization {
protected:
	AttributeHolder atts;
	vector<int> linkw;
public:
	bool CheckFlag(Attributes tocheck) { return atts.CheckFlag(tocheck); }
	void SetFlag(Attributes a) { atts.SetFlag(a); }
	void UnSetFlag(Attributes a) { atts.UnSetFlag(a); }
	void SetCode(int code) { atts.SetCode(code); }
	int GetFlag() { return atts.GetFlag(); }
	void Fill(int sz);
	void Erase();
};




class INodeFileNoteRealisation :public StandartFileNoteRealisation {
protected:
	virtual void AbstractFlag() = 0;
	NameRealisation* nme;
	AttributeHolder atts;
	INodeFileRealization* file;
public:
	void SetName(string nm) {
		nme->SetName(nm);
	}
	string GetName() {
		return nme->GetName();
	}
	void SetFlag(Attributes a) { file->SetFlag(a); }
	void UnSetFlag(Attributes a) { file->UnSetFlag(a); }
	bool CheckFlag(Attributes a) { return file->CheckFlag(a); }
	void SetCode(int code) { file->SetCode(code); }

	void Open() { file->Open(); }
	void Close() { file->Close(); }
	void Fill(int size) { file->Fill(size); }
	void Erase() { file->Erase(); }
	void Delete() { file->Unlink(); }

	int Link() { file->Link(); return 1; }
	int GetSize() { return file->GetSize(); }
	int GetFlagCode() { return file->GetFlag(); }
	IFileRealization* GetVal(bool& ValReturned) { ValReturned = true; return file; }
	void SetVal(IFileRealization* f) {
		file->Unlink();
		file = (INodeFileRealization*)f;
	}

};


class StandartLabel :public FSNote {
protected:
	virtual void AbstractFlag() = 0;
	NameRealisation* nme;
	AttributeHolder atts;
	string path;
public:
	void SetLinkedObject(string p) { path = p; }

	void SetName(string nm) {
		nme->SetName(nm);
	}
	string GetName() {
		return nme->GetName();
	}
	void SetFlag(Attributes a) { atts.SetFlag(a); }
	void UnSetFlag(Attributes a) { atts.UnSetFlag(a); }
	bool CheckFlag(Attributes a) { return atts.CheckFlag(a); }
	void SetCode(int code) { atts.SetCode(code); }

	void Open();
	void Close();
	void Fill(int size);
	void Erase();
	void Delete();
	void DestructLabel() {
		delete nme;

	}
	int Link();
	int GetSize();
	int GetFlagCode() { return atts.GetFlag(); }
	IFileRealization* GetVal(bool& ValReturned);
	void SetVal(IFileRealization* f);

};


class FileSystem {
protected:
	FSNodeFactory* factory;
	FSNote* Root;

	void CreateFile(FSNote* Host, string name, int code = 0, int size = 0, IFileRealization* f = nullptr);
	void DeleteFile(FSNote* Host, string name);
	void CreateDir(FSNote* Host, string name, int code = 0);
	void DeleteDir(FSNote* Host, string name);
	void OpenFile(FSNote* Host, string name);
	void CloseFile(FSNote* Host, string name);
	void ls(FSNote* Host);
	void DeleteLabelByPath(string sourcePath);
	void FindFileByTemplate(regex tmp, FSNote* DirToStart, string PrevIterPath);
public:
	FSNote* FindNoteByPath(string path, bool& FFounded, string& name);
	FileSystem() {};
	void SetFactory(FSNodeFactory* fct) { factory = fct; }
	void Init() {
		Root = factory->CreateDirNote();
		CreateDir(Root, "Rt");
		Root->SetFlag(FDir);
	}
	FileSystem(FSNodeFactory* fct) :factory(fct) {
		Init();
	};
	void CreateFileByPath(string path, int size = 0, int code = 0, IFileRealization* f = nullptr);
	void DeleteFileByPath(string path);
	void CreateDirByPath(string path, int code = 0);
	void DeleteDirByPath(string path);

	void lsByPath(string path);
	void OpenFileByPath(string path);
	void CloseFileByPath(string path);
	void SetFileFlagByPath(string path, Attributes att);
	void UnSetFileFlagByPath(string path, Attributes att);
	bool CheckFileFlagByPath(string path, Attributes att);
	void CopyFileByPath(string f1Path, string f2Path);
	void CreateHardLinkByPath(string sourcePath, string TargetPath);
	void CreateLabelByPath(string sourcePath, string TargetPath);

	void CopyDirByPath(string f1Path, string f2Path);
	void FindFileViaTemp(string temp);
	void DefragmentateDisk();
	void TestZone();
};


class HeapNameLabel :public StandartLabel {
protected:
	void AbstractFlag() { cout << "flag defused\n"; }
public:
	HeapNameLabel() { nme = new HeapNameContainer(); }
	FSNote* Copy() {
		HeapNameLabel* copie = new HeapNameLabel();
		copie->nme = new HeapNameContainer();
		copie->SetName(GetName());
		copie->atts = atts;
		copie->path = path;
		return (FSNote*)copie;

	}
};

class FullNameLabel :public StandartLabel {
protected:
	void AbstractFlag() { cout << "flag defused\n"; }
public:
	FullNameLabel() { nme = new BruteNameContainer(); }
	FSNote* Copy() {
		FullNameLabel* copie = new FullNameLabel();
		copie->nme = new BruteNameContainer();
		copie->SetName(GetName());
		copie->atts = atts;
		copie->path = path;
		return (FSNote*)copie;
	}
};

class ContFileNoteWithFullName :public StandartFileNoteRealisation {
protected:
	void AbstractFlag() { cout << "itsfine\n"; }
public:
	ContFileNoteWithFullName() { file = new ContFileRealization(); nme = new BruteNameContainer(); }
	int GetFirstDiskClaster(bool& ReturnedRealValue) { return file->GetFirstDiskClaster(ReturnedRealValue); }
	FSNote* Copy() {
		ContFileNoteWithFullName* copy = new ContFileNoteWithFullName();
		copy->file = new ContFileRealization();
		copy->file->Fill(file->GetSize());
		copy->nme = new BruteNameContainer();
		copy->SetName(nme->GetName());
		copy->atts = atts;
		return (FSNote*)copy;
	}
};

class ContFileNoteWithPointerName :public StandartFileNoteRealisation {
protected:
	void AbstractFlag() { cout << "itsfine\n"; }
public:
	ContFileNoteWithPointerName() { file = new ContFileRealization(); nme = new HeapNameContainer(); }
	int GetFirstDiskClaster(bool& ReturnedRealValue) { return file->GetFirstDiskClaster(ReturnedRealValue); }
	FSNote* Copy() {
		ContFileNoteWithPointerName* copy = new ContFileNoteWithPointerName();
		copy->file = new ContFileRealization();
		copy->file->Fill(file->GetSize());
		copy->nme = new HeapNameContainer();
		copy->SetName(nme->GetName());
		copy->atts = atts;
		return (FSNote*)copy;
	}
};

class LinkListFileNoteWithFullName :public StandartFileNoteRealisation {
protected:
	void AbstractFlag() { cout << "itsfine\n"; }
public:
	LinkListFileNoteWithFullName() { file = new LinkLIstRealization(), nme = new BruteNameContainer(); }
	FSNote* Copy() {
		LinkListFileNoteWithFullName* copy = new LinkListFileNoteWithFullName();
		copy->file = new LinkLIstRealization();
		copy->file->Fill(file->GetSize());
		copy->nme = new BruteNameContainer();
		copy->SetName(nme->GetName());
		copy->atts = atts;
		return (FSNote*)copy;
	}
};

class LinkListFileNoteWithPointerName :public StandartFileNoteRealisation {
protected:
	void AbstractFlag() { cout << "itsfine\n"; }
public:
	LinkListFileNoteWithPointerName() { file = new LinkLIstRealization(), nme = new HeapNameContainer(); }
	FSNote* Copy() {
		LinkListFileNoteWithPointerName* copy = new LinkListFileNoteWithPointerName();
		copy->file = new LinkLIstRealization();
		copy->file->Fill(file->GetSize());
		copy->nme = new HeapNameContainer();
		copy->SetName(nme->GetName());
		copy->atts = atts;
		return (FSNote*)copy;
	}
};

class FatFileNoteWithFullName :public StandartFileNoteRealisation {
protected:
	void AbstractFlag() { cout << "itsfine\n"; }
public:
	FatFileNoteWithFullName() { file = new FATRealizaton(), nme = new BruteNameContainer(); }

	FSNote* Copy() {
		FatFileNoteWithFullName* copy = new FatFileNoteWithFullName();
		copy->file = new FATRealizaton();
		copy->file->Fill(file->GetSize());
		copy->nme = new BruteNameContainer();
		copy->SetName(nme->GetName());
		copy->atts = atts;
		return (FSNote*)copy;
	}
};

class FatFileNoteWithPointerName :public StandartFileNoteRealisation {
protected:
	void AbstractFlag() { cout << "itsfine\n"; }
public:
	FatFileNoteWithPointerName() { file = new FATRealizaton(), nme = new HeapNameContainer(); }
	FSNote* Copy() {
		FatFileNoteWithPointerName* copy = new FatFileNoteWithPointerName();
		copy->file = new FATRealizaton();
		copy->file->Fill(file->GetSize());
		copy->nme = new BruteNameContainer();
		copy->SetName(nme->GetName());
		copy->atts = atts;
		return (FSNote*)copy;
	}
};

class InodeFileNoteWithFullName :public INodeFileNoteRealisation {
protected:
	void AbstractFlag() { cout << "itsfine\n"; }
public:
	InodeFileNoteWithFullName() { file = new INodeFileRealization(), nme = new BruteNameContainer(); }
	FSNote* Copy() {
		InodeFileNoteWithFullName* copy = new InodeFileNoteWithFullName();
		copy->file = new INodeFileRealization();
		copy->file->SetCode(file->GetFlag());
		copy->file->Fill(file->GetSize());
		copy->nme = new BruteNameContainer();
		copy->SetName(nme->GetName());
		copy->atts = atts;
		return (FSNote*)copy;
	}
};

class INodeFileNoteWithPointerName :public INodeFileNoteRealisation {
protected:
	void AbstractFlag() { cout << "itsfine\n"; }
public:
	INodeFileNoteWithPointerName() { file = new INodeFileRealization(), nme = new BruteNameContainer(); }
	FSNote* Copy() {
		INodeFileNoteWithPointerName* copy = new INodeFileNoteWithPointerName();
		copy->file = new INodeFileRealization();
		copy->file->SetCode(file->GetFlag());
		copy->file->Fill(file->GetSize());
		copy->nme = new BruteNameContainer();
		copy->SetName(nme->GetName());
		copy->atts = atts;
		return (FSNote*)copy;
	}
};


