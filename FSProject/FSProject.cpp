#include "FS.h"

Buffer HDD;
Buffer RAM;
FileSystem* FS = new FileSystem();
Buffer FAT;


int Buffer::GetFirstFree(int len, bool FromEnd) {
	for (int i = 0; i < size; i++) {
		if (_val[i] == nulv) {
			int counter = 0;
			for (int j = i; j < size; j++) {
				if (_val[j] != nulv) {
					i = j;
					break;
				}
				counter++;
				if (counter >= len) { return i; }
			}
		}
	}
	return -1;
}

void RamDisplay::Push(int sz) {
	if (pos != -1) {
		Clear();
	}
	size = sz;
	pos = RAM.GetFirstFree(size);
	RAM.Write(pos, size);
}
void RamDisplay::Clear() {

	if (pos != -1) {
		RAM.Clear(pos, size);
		size = 0;
		pos = -1;
	}
}


void ContFileRealization::Fill(int s) {
	if (pos != -1) {
		Close();
	}
	size = s;
	pos = HDD.GetFirstFree(size);
	HDD.Write(pos, size);
}
void ContFileRealization::Erase() {
	if (pos != -1) {
		HDD.Clear(pos, size);
		size = 0;
		pos = -1;
	}
}

void IFileRealization::Open() {
	rd.Push(size);
}
void IFileRealization::Close() {
	rd.Clear();
}

void LinkLIstRealization::Fill(int sz) {
	size = sz;
	Erase();
	First = new Link1();
	Link1* cur = First;
	for (int i = 0; i < sz; i++) {
		cur->Next = new Link1();
		cur = cur->Next;
		cur->val = HDD.GetFirstFree(1);
		HDD.Write(cur->val, 1);
	}
}
void LinkLIstRealization::Erase() {
	if (First == nullptr) { return; }
	Link1* cur = First;
	if (cur->Next == nullptr) { return; }
	while (cur->Next != nullptr) {
		cur = cur->Next;
		HDD.Clear(cur->val, 1);
	}
	First->Delete();
	delete First;
}

void FATRealizaton::Fill(int sz) {
	size = sz;
	Erase();

	startind = FAT.GetFirstFree();
	int cur = startind;
	FAT.Write(cur, 1, -3);
	int nxt = -2;
	for (int i = 0; i < sz; i++) {
		nxt = FAT.GetFirstFree(1);
		FAT.Write(cur, 1, nxt);
		HDD.Write(cur, 1);
		FAT.Write(nxt, 1, -3);

		cur = nxt;
	}
	HDD.Write(nxt, 1);
	FAT.Write(nxt, 1, -1);
}
void FATRealizaton::Erase() {
	if (startind == -2 || startind == -1) { return; }
	else {
		int cur = startind;
		int nxt = FAT.GetVal(cur);
		FAT.Clear(cur, 1);
		HDD.Clear(cur, 1);
		cur = nxt;
		while (FAT.GetVal(cur) != -1) {
			nxt = FAT.GetVal(cur);
			FAT.Clear(cur, 1);
			HDD.Clear(cur, 1);
			cur = nxt;
		}
		FAT.Clear(cur, 1);
		HDD.Clear(cur, 1);
	}
}

void INodeFileRealization::Fill(int sz) {
	Erase();
	size = sz;
	for (int i = 0; i < size; i++) {
		int x = HDD.GetFirstFree(1);
		linkw.push_back(x);
		HDD.Write(x, 1);

	}
}
void INodeFileRealization::Erase() {

	for (auto x : linkw) {
		HDD.Clear(x, 1);
	}
	linkw.clear();
}

void StandartDirNoteRealisation::DeleteChild(string name, int code) {
	for (int i = 0; i < children.size(); i++) {
		if (children[i]->GetName() == name) {
			if (code == 0) {
				children[i]->Delete();
				delete children[i];
				children.erase(children.begin() + i);
				return;
			}
			else if (code == 1) {
				((StandartLabel*)children[i])->DestructLabel();
				delete children[i];
				children.erase(children.begin() + i);
				return;
			}
		}
	}
	cout << "No such file or Directory\n";
}
int StandartDirNoteRealisation::AddChild(FSNote* toAdd) {
	for (auto x : children) {
		if (x->GetName() == toAdd->GetName()) {
			cout << "file or Directory with such name already exists\n";
			return 0;
		}
	}
	children.push_back(toAdd);
	return 1;
}
FSNote* StandartDirNoteRealisation::GetNoteFromName(string name, bool& Founded) {
	for (auto x : children) {
		if (x->GetName() == name) {
			Founded = true;
			return x;
		}
	}
	Founded = false;
	return nullptr;
}

void StandartLabel::Open() {
	bool Founded = false;
	string FileName;
	FSNote* Directory = FS->FindNoteByPath(path, Founded, FileName);
	if (Founded) {
		FSNote* FileNote = Directory->GetNoteFromName(FileName, Founded);
		if (Founded) {
			FileNote->Open();
		}
	}
}
void StandartLabel::Close() {
	bool Founded = false;
	string FileName;
	FSNote* Directory = FS->FindNoteByPath(path, Founded, FileName);
	if (Founded) {
		FSNote* FileNote = Directory->GetNoteFromName(FileName, Founded);
		if (Founded) {
			FileNote->Close();
		}
	}
}
void StandartLabel::Fill(int size) {
	bool Founded = false;
	string FileName;
	FSNote* Directory = FS->FindNoteByPath(path, Founded, FileName);
	if (Founded) {
		FSNote* FileNote = Directory->GetNoteFromName(FileName, Founded);
		if (Founded) {
			FileNote->Fill(size);
		}
	}

}
void StandartLabel::Erase() {
	bool Founded = false;
	string FileName;
	FSNote* Directory = FS->FindNoteByPath(path, Founded, FileName);
	if (Founded) {
		FSNote* FileNote = Directory->GetNoteFromName(FileName, Founded);
		if (Founded) {
			FileNote->Erase();
		}
	}
}
void StandartLabel::Delete() {
	cout << "Label cannot delete source file, use delete function with the next path:\n";
	cout << path << "\n";
}
int StandartLabel::Link() {
	bool Founded = false;
	string FileName;
	FSNote* Directory = FS->FindNoteByPath(path, Founded, FileName);
	if (Founded) {
		FSNote* FileNote = Directory->GetNoteFromName(FileName, Founded);
		if (Founded) {
			return FileNote->Link();
		}
	}
	return 0;
}
int StandartLabel::GetSize() {
	bool Founded = false;
	string FileName;
	FSNote* Directory = FS->FindNoteByPath(path, Founded, FileName);
	if (Founded) {
		FSNote* FileNote = Directory->GetNoteFromName(FileName, Founded);
		if (Founded) {
			return FileNote->GetSize();
		}
	}
}
IFileRealization* StandartLabel::GetVal(bool& ValReturned) {
	bool Founded = false;
	string FileName;
	FSNote* Directory = FS->FindNoteByPath(path, Founded, FileName);
	if (Founded) {
		FSNote* FileNote = Directory->GetNoteFromName(FileName, Founded);
		if (Founded) {
			return FileNote->GetVal(ValReturned);
		}

	}
	ValReturned = false;
	return nullptr;
}
void StandartLabel::SetVal(IFileRealization* f) {
	bool Founded = false;
	string FileName;
	FSNote* Directory = FS->FindNoteByPath(path, Founded, FileName);
	if (Founded) {
		FSNote* FileNote = Directory->GetNoteFromName(FileName, Founded);
		if (Founded) {
			FileNote->SetVal(f);
		}
	}

}



void FileSystem::CreateFile(FSNote* Host, string name, int code, int size, IFileRealization* f) {
	FSNote* nf = factory->CreateFileNote();
	nf->SetName(name);
	if (Host->AddChild(nf) != 0) {
		nf->SetCode(code);
		nf->Fill(size);
		if (f != nullptr) {
			nf->SetVal(f);
		}
	}
	else {
		nf->Delete();
		delete nf;
	}
}
void FileSystem::DeleteFile(FSNote* Host, string name) {
	Host->DeleteChild(name);

}


void FileSystem::CreateDir(FSNote* Host, string name, int code) {
	FSNote* fn = factory->CreateDirNote();
	fn->SetName(name);
	if (Host->AddChild(fn) != 0) {
		fn->SetCode(code);
		fn->SetFlag(FDir);
	}
	else {
		fn->Delete();
		delete fn;
	}
}
void FileSystem::DeleteDir(FSNote* Host, string name) {
	Host->DeleteChild(name);
}


void FileSystem::OpenFile(FSNote* Host, string name) {
	bool Founded;
	FSNote* file = Host->GetNoteFromName(name, Founded);
	if (Founded) {
		file->Open();
	}
}
void FileSystem::CloseFile(FSNote* Host, string name) {
	bool Founded;
	FSNote* file = Host->GetNoteFromName(name, Founded);
	if (Founded) {
		file->Close();
	}
}


void FileSystem::ls(FSNote* Host) {
	for (auto x : Host->GetChildren()) {
		cout << x->GetName() << "\n";
	}
}
void FileSystem::FindFileByTemplate(regex tmp, FSNote* DirToStart, string PrevIterPath) {
	PrevIterPath = PrevIterPath + "/" + DirToStart->GetName();
	for (auto x : DirToStart->GetChildren()) {
		if (regex_match(x->GetName(), tmp)) {
			cout << PrevIterPath + "/" + x->GetName() << "\n";
		}
		else {
			if (x->CheckFlag(FDir) == 1) {
				FindFileByTemplate(tmp, x, PrevIterPath);
			}
		}
	}

}


FSNote* FileSystem::FindNoteByPath(string path, bool& FFounded, string& name) {
	PathHolder p(path);
	FSNote* curdir = Root;
	while (!p.NextIsLast()) {
		string cnm = p.GetCurrent();
		bool Founded = false;
		FSNote* check = curdir->GetNoteFromName(cnm, Founded);
		if (!Founded) { FFounded = false; return nullptr; }
		else {
			curdir = check;
			p.Next();
		}
	}
	bool fd = false;
	FSNote* check = curdir->GetNoteFromName(p.GetCurrent(), fd);
	if (fd) {
		FFounded = true;
		name = p.GetCurrent();
		return curdir;
	}
	else {
		FFounded = false;
		return nullptr;
	}
}


void FileSystem::CreateFileByPath(string path, int size, int code, IFileRealization* f) {
	PathHolder p(path);
	FSNote* curdir = Root;
	while (!p.NextIsLast()) {
		string cnm = p.GetCurrent();
		bool Founded = false;
		FSNote* check = curdir->GetNoteFromName(cnm, Founded);
		if (!Founded) {
			CreateDir(curdir, cnm);
		}
		else {
			curdir = check;
			p.Next();
		}
	}
	CreateFile(curdir, p.GetCurrent(), code, size, f);

}
void FileSystem::DeleteFileByPath(string path) {
	bool Founded = false;
	string name;
	FSNote* fl = FindNoteByPath(path, Founded, name);

	if (Founded) {
		FSNote* ft = fl->GetNoteFromName(name, Founded);
		if (Founded) {
			if (ft->CheckFlag(FDir) == 0) {

				DeleteFile(fl, name);
			}
			else {
				cout << "Directories can not be deleted via this function\n";
			}
		}
	}
	else {
		cout << "No such file, or file have wrong code\n";
	}
}


void FileSystem::CreateDirByPath(string path, int code) {
	PathHolder p(path);
	FSNote* curdir = Root;
	while (!p.NextIsLast()) {
		string cnm = p.GetCurrent();
		bool Founded = false;
		FSNote* check = curdir->GetNoteFromName(cnm, Founded);
		if (!Founded) {
			CreateDir(curdir, cnm);
		}
		else {
			curdir = check;
			p.Next();
		}
	}
	CreateDir(curdir, p.GetCurrent(), code);
}
void FileSystem::DeleteDirByPath(string path) {
	bool Founded = false;
	string name;
	FSNote* fl = FindNoteByPath(path, Founded, name);

	if (Founded) {
		FSNote* ft = fl->GetNoteFromName(name, Founded);
		if (Founded) {
			if (ft->CheckFlag(FDir) == 1)
			{
				DeleteDir(fl, name);
			}
			else {
				cout << "Only directories can be deleted via this function\n";
			}
		}
	}
	else {
		cout << "No such directory, or directory have wrong code\n";
	}
}


void FileSystem::lsByPath(string path) {
	bool Founded = false;
	string name;
	FSNote* fl = FindNoteByPath(path, Founded, name);

	if (Founded) {
		FSNote* ft = fl->GetNoteFromName(name, Founded);
		if (Founded) {
			if (ft->CheckFlag(FDir) == 1) {
				ls(ft);
			}
			else {
				cout << "This function not allowed for files\n";
			}
		}
	}
	else {
		cout << "No such file or directory, or directory have wrong code\n";
	}

}
void FileSystem::OpenFileByPath(string path) {
	bool Founded = false;
	string name;
	FSNote* fl = FindNoteByPath(path, Founded, name);

	if (Founded) {

		FSNote* ft = fl->GetNoteFromName(name, Founded);
		if (Founded) {
			if (ft->CheckFlag(FDir) == 0) {
				OpenFile(fl, name);
			}
			else {
				cout << "This function not allowed for directories\n";
			}
		}
	}
	else {
		cout << "No such file or directory, or directory have wrong code\n";
	}
}
void FileSystem::CloseFileByPath(string path) {
	bool Founded = false;
	string name;
	FSNote* fl = FindNoteByPath(path, Founded, name);

	if (Founded) {
		FSNote* ft = fl->GetNoteFromName(name, Founded);
		if (Founded) {
			if (ft->CheckFlag(FDir) == 0) {
				CloseFile(fl, name);
			}
			else {
				cout << "This function is not allowed for directories\n";
			}
		}
	}
	else {
		cout << "No such file or file have wrong code\n";
	}
}
void FileSystem::CopyFileByPath(string f1Path, string f2Path) {
	bool SourceFileExist = 0;
	string SourceFileName;
	FSNote* DirectoryWithSourceFile = FindNoteByPath(f1Path, SourceFileExist, SourceFileName);
	if (!SourceFileExist) {
		cout << "Error while coping file, Source File not exist\n";
		return;
	}
	bool TargetFileExist = 0;
	string TargetFileName;
	FSNote* DirectoryWithTargetFile = FindNoteByPath(f2Path, TargetFileExist, TargetFileName);
	if (TargetFileExist) {
		cout << "File with this path already exist, cant create 2 files with the same names\n";
		return;
	}
	PathHolder p(f2Path);
	FSNote* curdir = Root;
	while (!p.NextIsLast()) {
		string cnm = p.GetCurrent();
		bool Founded = false;
		FSNote* check = curdir->GetNoteFromName(cnm, Founded);
		if (!Founded) {
			CreateDir(curdir, cnm);
		}
		else {
			curdir = check;
			p.Next();
		}
	}
	TargetFileName = p.GetCurrent();
	DirectoryWithTargetFile = curdir;
	FSNote* SourceFile = DirectoryWithSourceFile->GetNoteFromName(SourceFileName, SourceFileExist);
	if (SourceFileExist) {
		FSNote* NNote = SourceFile->Copy();
		NNote->SetName(TargetFileName);
		int z = DirectoryWithTargetFile->AddChild(NNote);
		if (z == 1) { return; }
		if (z != 1) {
			cout << "unexpected error, probably file already eist\n";
		}

	}
	else {
		cout << "SourceFileNotFound\n";
	}

}
void FileSystem::CreateHardLinkByPath(string sourcePath, string TargetPath) {
	bool SourceFound = false;
	string Sourcename;
	FSNote* SourceDir = FindNoteByPath(sourcePath, SourceFound, Sourcename);
	if (!SourceFound) {
		cout << "No such source file\n";
		return;
	}
	else {
		bool TargExist = false;
		string targname;
		FSNote* TargetDir = FindNoteByPath(TargetPath, TargExist, targname);
		if (TargExist) {
			cout << "targetFile with such name already exist, please use this methode with second path to unexisting file\n";
			return;
		}
		else {


			FSNote* SourceFile = SourceDir->GetNoteFromName(Sourcename, SourceFound);
			if (SourceFound) {
				if (SourceFile->CheckFlag(FDir) == 0) {
					bool x = false;
					IFileRealization* f = SourceFile->GetVal(x);
					if (x) {
						CreateFileByPath(TargetPath, 0, 0, f);
						SourceFile->Link();
					}
					else {
						cout << "unexpected error while linking files\n";
						return;
					}

				}
				else {
					cout << "Cant copy directory via File copy procedure\n";
					return;
				}
			}
		}
	}
}

void FileSystem::SetFileFlagByPath(string math, Attributes a) {
	bool Founede = false;
	string fname;
	FSNote* TargetDir = FindNoteByPath(math, Founede, fname);
	if (Founede) {
		if (a != FDir && a != FLabel) {
			FSNote* targFIle = TargetDir->GetNoteFromName(fname, Founede);
			if (Founede) {
				targFIle->SetFlag(a);
			}
			else {
				cout << "Unexpected error while Setting a flag\n";
			}
		}
		else {
			cout << "this type of flags can not be placed by user\n";
		}
	}
	else {
		cout << "No such file or directory\n";
	}
}
void FileSystem::UnSetFileFlagByPath(string math, Attributes a) {
	bool Founede = false;
	string fname;
	FSNote* TargetDir = FindNoteByPath(math, Founede, fname);
	if (Founede) {
		if (a != FDir && a != FLabel) {
			FSNote* targFIle = TargetDir->GetNoteFromName(fname, Founede);
			if (Founede) {
				targFIle->UnSetFlag(a);
			}
			else {
				cout << "Unexpected error while unsetting a flag\n";
			}
		}
		else {
			cout << "this type of flags cant be deleted by user\n";
		}
	}
	else {
		cout << "No such file or directory\n";
	}
}
bool FileSystem::CheckFileFlagByPath(string math, Attributes a) {
	bool Founede = false;
	string fname;
	FSNote* TargetDir = FindNoteByPath(math, Founede, fname);
	if (Founede) {

		FSNote* targFIle = TargetDir->GetNoteFromName(fname, Founede);
		if (Founede) {
			return targFIle->CheckFlag(a);
		}
		else {
			cout << "unexpected error while checking flag\n";
		}


	}
	else {
		cout << "No such file or directory\n";
		return -1;
	}
}
void FileSystem::CreateLabelByPath(string sourcePath, string TargetPath) {
	bool SourceFound = false;
	string Sourcename;
	FSNote* SourceDir = FindNoteByPath(sourcePath, SourceFound, Sourcename);
	if (!SourceFound) {
		cout << "No such source file\n";
		return;
	}
	else {
		bool TargExist = false;
		string targname;
		FSNote* TargetDir = FindNoteByPath(TargetPath, TargExist, targname);
		if (TargExist) {
			cout << "targetFile with such name already exist, please use this methode with second path to unexisting file\n";
			return;
		}
		else {


			FSNote* SourceFile = SourceDir->GetNoteFromName(Sourcename, SourceFound);
			if (SourceFound) {
				if (SourceFile->CheckFlag(FDir) == 0) {

					
					PathHolder p(TargetPath);
					FSNote* curdir = Root;
					while (!p.NextIsLast()) {
						string cnm = p.GetCurrent();
						bool Founded = false;
						FSNote* check = curdir->GetNoteFromName(cnm, Founded);
						if (!Founded) {
							CreateDir(curdir, cnm);
						}
						else {
							curdir = check;
							p.Next();
						}
					}
					
					StandartLabel* z = (StandartLabel*)factory->CreateLabel();
					z->SetLinkedObject(sourcePath);
					z->SetName(p.GetCurrent());
					z->SetFlag(FLabel);
					int Good = curdir->AddChild(z);
					if (!Good) {
						cout << "unexpected error while creating label\n";
					}

				}
				else {
					cout << "Label creating procedure only defined for files, not dirs\n";
					return;
				}
			}
		}
	}
}
void FileSystem::DeleteLabelByPath(string sourcePath) {
	bool Founded = false;
	string filename;
	FSNote* DirectoryWithFile = FindNoteByPath(sourcePath, Founded, filename);
	if (Founded) {
		bool x = false;
		StandartLabel* file = (StandartLabel*)DirectoryWithFile->GetNoteFromName(filename, x);
		if (x) {
			if (file->CheckFlag(FLabel)) {
				DirectoryWithFile->DeleteChild(filename, 1);


			}
			else {
				cout << "File is not a Label\n";
			}
		}
	}
	else {
		cout << "no such file or directory\n";
	}
}
void FileSystem::FindFileViaTemp(string temp) {
	regex rx(temp);
	bool found;
	FSNote* rt = Root->GetNoteFromName("Rt", found);
	FindFileByTemplate(rx, rt, "");
}



void TakeAllFiles(FSNote* ToStart, vector<ContFileRealization*>& storage) {
	if (ToStart->CheckFlag(FDir) == 1) {
		for (FSNote* File : ToStart->GetChildren()) {
			if (File->CheckFlag(FDir) == 1) {
				TakeAllFiles(File, storage);
			}
			else {
				bool found = false;
				ContFileRealization* x = (ContFileRealization*)(File->GetVal(found));
				if (found) {
					int z = x->GetFirstDiskClaster(found);
					if (found) {
						if (z != -1) {
							storage.push_back(x);
						}
					}
				}
			}
		}
	}
}
void FileSystem::DefragmentateDisk() {
	if (factory->AllowDefragmentation()) {
		vector<ContFileRealization*> Files;
		TakeAllFiles(Root, Files);
		sort(Files.begin(), Files.end(), [](ContFileRealization* f1, ContFileRealization* f2) {
			bool d;
			return f1->GetFirstDiskClaster(d) < f2->GetFirstDiskClaster(d);
			});
		vector<ContFileRealization*> PureFiles;
		int lastVal = -1;
		bool fool;
		for (int i = 0; i < Files.size(); i++) {
			if (Files[i]->GetFirstDiskClaster(fool) != lastVal) {
				PureFiles.push_back(Files[i]);
				lastVal = Files[i]->GetFirstDiskClaster(fool);
			}
		}
		int curfree = 0;
		for (int i = 0; i < PureFiles.size(); i++) {
			int delta = PureFiles[i]->GetFirstDiskClaster(fool) - curfree;
			PureFiles[i]->Shift(delta);
			curfree += PureFiles[i]->GetSize();
		}
	}
	else {
		cout << "defragmentation procedure not allowed for this type of file storing\n";
	}
}


class ContFileRealWithFullNameFactory :public FSNodeFactory {
public:
	FSNote* CreateFileNote() { return new ContFileNoteWithFullName(); }
	FSNote* CreateDirNote() { return new FullnameDirNote(); }
	FSNote* CreateLabel() { return new FullNameLabel(); }
	bool AllowDefragmentation() { return true; }
};

class ContFileRealWithHeapNameFactory :public FSNodeFactory {
public:
	FSNote* CreateFileNote() { return new ContFileNoteWithPointerName(); }
	FSNote* CreateDirNote() { return new HeapNameDirNote(); }
	FSNote* CreateLabel() { return new HeapNameLabel(); }
	bool AllowDefragmentation() { return true; }
};

class LinkListFileRealWithFullNameFactory :public FSNodeFactory {
	FSNote* CreateFileNote() { return new LinkListFileNoteWithFullName(); }
	FSNote* CreateDirNote() { return new FullnameDirNote(); }
	FSNote* CreateLabel() { return new FullNameLabel(); }

};

class LinkListFileRealWithHeapNameFactory :public FSNodeFactory {
	FSNote* CreateFileNote() { return new LinkListFileNoteWithPointerName(); }
	FSNote* CreateDirNote() { return new HeapNameDirNote(); }
	FSNote* CreateLabel() { return new HeapNameLabel(); }

};

class FATFileRealWithFullNameFactory :public FSNodeFactory {
	FSNote* CreateFileNote() { return new FatFileNoteWithFullName(); }
	FSNote* CreateDirNote() { return new FullnameDirNote(); }
	FSNote* CreateLabel() { return new FullNameLabel(); }
};

class FATFileRealWithHeapNameFactory :public FSNodeFactory {
	FSNote* CreateFileNote() { return new FatFileNoteWithPointerName(); }
	FSNote* CreateDirNote() { return new HeapNameDirNote(); }
	FSNote* CreateLabel() { return new HeapNameLabel(); }
};

class INodeFileRealWithFullNameFactory :public FSNodeFactory {
	FSNote* CreateFileNote() { return new InodeFileNoteWithFullName(); }
	FSNote* CreateDirNote() { return new FullnameDirNote(); }
	FSNote* CreateLabel() { return new FullNameLabel(); }
};

class INodeFileRealWithHeapNameFactory :public FSNodeFactory {
	FSNote* CreateFileNote() { return new INodeFileNoteWithPointerName(); }
	FSNote* CreateDirNote() { return new HeapNameDirNote(); }
	FSNote* CreateLabel() { return new HeapNameLabel(); }
};

void ContFileRealization::Shift(int countofFree) {
	HDD.Clear(pos, size);
	pos -= countofFree;
	HDD.Write(pos, size);
}

FSNodeFactory* factory = new LinkListFileRealWithFullNameFactory();





void FileSystem::TestZone() {
}


void Init() {

	FS->SetFactory(factory);
	FS->Init();
	for (int i = 0; i < MassiveSize; i++) {
		FAT = Buffer(-2);
	}
}

void FileCreateonTest() {
	FS->CreateFileByPath("Rt/test1/File",10);
	FS->CreateFileByPath("Rt/test1/File",10); // not allowed
	FS->CreateFileByPath("Rt/test1/File1",10);
	HDD.DampOnScreen();
	FS->DeleteFileByPath("Rt/test1/File");
	FS->DeleteFileByPath("Rt/test1/File1");
}
void FileDeleteTest() {
	FS->CreateFileByPath("Rt/test2/File", 10);
	FS->CreateFileByPath("Rt/test2/File1", 5);
	FS->lsByPath("Rt/test2");
	HDD.DampOnScreen(); // file found in ram
	FS->DeleteFileByPath("Rt/test2/File");
	FS->lsByPath("Rt/test2");
	HDD.DampOnScreen(); // file cleared from ram

	FS->DeleteFileByPath("Rt/test2/File"); // no result
	FS->DeleteFileByPath("Rt/test2/File"); // no result
	FS->DeleteDirByPath("RT/test2/UnexistingFile");//no result
	FS->DeleteFileByPath("Rt/test2");// not allowed, deleting a dir from a file funtion
}
void p() {
	FS->CreateFileByPath("Rt/test3/File",10);
	FS->OpenFileByPath("Rt/test3/File");
	RAM.DampOnScreen();
	FS->DeleteFileByPath("Rt/test3/File"); // File closed and deleted from HDD
	RAM.DampOnScreen();
}
void CloseFileTest()    {
	FS->CreateFileByPath("Rt/test4/File",10);
	FS->OpenFileByPath("Rt/test4/File");
	RAM.DampOnScreen();
	FS->CloseFileByPath("Rt/test4/File");
	RAM.DampOnScreen();
	FS->DeleteFileByPath("Rt/test4/File");
	FS->OpenFileByPath("Rt/AAA");//no result
	FS->OpenFileByPath("Rt/test4");//no result
}

void CreateDirTest() {
	FS->CreateDirByPath("Rt/test5/TestDIr");
	FS->lsByPath("Rt/test5");
}
void DeleteDirTest() {
	FS->CreateDirByPath("Rt/test5/TestDIr");
	FS->CreateFileByPath("Rt/test5/TestDIr/F1",1);
	FS->CreateFileByPath("Rt/test5/TestDIr/F2",1);
	FS->CreateFileByPath("Rt/test5/TestDIr/F3",1);
	FS->CreateFileByPath("Rt/test5/TestDIr/F4",1);
	FS->CreateFileByPath("Rt/test5/TestDIr/F5",1);
	HDD.DampOnScreen();
	FS->lsByPath("Rt/test5/TestDIr");
	FS->DeleteDirByPath("Rt/test5/TestDIr");
	FS->lsByPath("Rt/test5/TestDIr");
	HDD.DampOnScreen();
	FS->lsByPath("Rt/test5");
}

void CopyFileTest() {
	FS->CreateFileByPath("Rt/test6/File1", 5);
	FS->CopyFileByPath("Rt/test6/File1","Rt/test6/File1"); // not Allowed
	FS->lsByPath("Rt/test6");
	HDD.DampOnScreen();
	FS->CopyFileByPath("Rt/test6/File1", "Rt/test6/File2");
	FS->lsByPath("Rt/test6");
	HDD.DampOnScreen();
}

void CopyDirTest() {
	FS->CreateFileByPath("Rt/test7/SourceDir/File1", 5);
	FS->CopyFileByPath("Rt/test7/SourceDir/File1", "Rt/test7/SourceDir/File2");
	FS->CopyFileByPath("Rt/test7/SourceDir", "Rt/test7/CopiedDir");
	HDD.DampOnScreen();
	FS->lsByPath("Rt/test7");
	FS->lsByPath("Rt/test7/CopiedDir");
}

void FileSearchTest() {
	FS->CreateFileByPath("Rt/test8/Directory1/File1.txt",5);
	FS->CreateFileByPath("Rt/test8/Dir2/File1.txt", 5);
	FS->CreateFileByPath("Rt/test8/Directory1/File1.dat", 5);
	FS->CreateFileByPath("Rt/test8/Dir3/File1.txt", 5);
	FS->FindFileViaTemp(R"(\w*.txt)");
}

void HardLinkCreation() {
	FS->CreateFileByPath("Rt/test9/File1", 5);
	FS->CreateHardLinkByPath("Rt/test9/File1", "Rt/test9");
	FS->CreateHardLinkByPath("Rt/test9/File1", "Rt/test9/File2");
	FS->OpenFileByPath("Rt/test9/File2");
	FS->OpenFileByPath("Rt/test9/File1");
	RAM.DampOnScreen();
	FS->DeleteFileByPath("Rt/test9/File2");
	HDD.DampOnScreen();
	FS->DeleteFileByPath("Rt/test9/File1");
	HDD.DampOnScreen();
}
void LabelCreationTest() {
	FS->CreateFileByPath("Rt/test10/Dir1/File1.txt", 5);
	FS->CreateLabelByPath("Rt/test10/Dir1/File1.txt", "Rt/test10/Dir1/File1.txt");//not allowed
	FS->CreateLabelByPath("Rt/test10/Dir1/File1.txt","Rt/test10/Dir1/Label");
	FS->OpenFileByPath("Rt/test10/Dir1/Label");
	RAM.DampOnScreen();

	FS->DeleteFileByPath("Rt/test10/Dir1/File1.txt");
	FS->OpenFileByPath("Rt/test10/Dir1/Label");
	FS->DeleteFileByPath("Rt/test10/Dir1/Label");
	RAM.DampOnScreen();

}

int main() {
	Init();
	FS->CreateFileByPath("Rt/Dir1/file1", 1);
	HDD.DampOnScreen();
	FS->DeleteFileByPath("Rt/Dir1/file1");
	HDD.DampOnScreen();
}