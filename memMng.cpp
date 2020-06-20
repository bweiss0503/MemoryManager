#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <sstream>

using namespace std;

struct FreeListNode
{
    uint32_t MAGIC_1;
    uint32_t size;
    FreeListNode * next;
    uint32_t MAGIC_2;
};

struct MallocHeader
{
	uint32_t MAGIC_1;
	uint32_t size;
	uint32_t MAGIC_2;
};

void checkCLArgs(vector<string> commandLine, int &z, int &s, int &c);
void * mallocHeap(int z, int s);
FreeListNode * init(void * heap, int z, int s, int c, vector <void *> &readySlabs);
FreeListNode * mallocDivide(int numSlabs, void * ptr, int slabSize, int &bytes, void * heap, vector <void *> &readySlabs);
void userDriver(void * heap, vector <FreeListNode *> fl, int z, int s, int c, vector <void *> readySlabs, vector <void *> takenSlabs, vector <void *> mallocedMem);
int roundUp(int numToRound);
void * myMalloc(void * ptr, int slabSize);
void freelist(void * heap, vector <void *> readySlabs, vector <FreeListNode*> fl);
string printAddress(void * address, void * heap);
int smallestFreeBlock(vector <FreeListNode *> fl);
int largestFreeBlock(vector <FreeListNode *> fl);
string magicCheck(void * ptr);
void slaballoc(void * heap, int slabSize, vector <FreeListNode *> &fl, vector <void *> &readySlabs, vector <void *> &takenSlabs, int numSlabs, int bytes);
void slabfree(string address, vector <void *> &takenSlabs, vector <void *> &readySlabs, void * heap);
void free(string address, vector <void *> &takenSlabs, vector <void *> &readySlabs, void * heap, vector <FreeListNode *> &fl);
void probe(string address, vector <void *> takenSlabs, vector <void *> readySlabs, vector <FreeListNode *> fl, void * heap);
void read(string address, vector <void *> takenSlabs, vector <void *> readySlabs, vector <FreeListNode *> fl, void * heap);
void write(string address, uint32_t, vector <void *> takenSlabs, vector <void *> readySlabs, vector <FreeListNode *> fl, void * heap);
void userMalloc(int size, vector <FreeListNode *> &fl, vector <void *> &mallocedMem);

//------------------------------------------------------------------
int main(int argc, char * argv[]) 
{
	vector<string> commandLine;
	argv++; 
	//Get rid of file location

	while(*argv)
	{
		commandLine.push_back(*(argv++)); 
		//push back each command line arguement into commandLine vector
	}

	int heapSpace = 64;
	int slabBytes = 256;
	int numSlabs = 8;
	//default values

	vector <void *> readySlabs;
	//free and ready slabs
	vector <void *> takenSlabs;
	//slabs claimed for use
	vector <void *> mallocedMem;
	//non slab allocated memory

	checkCLArgs(commandLine, heapSpace, slabBytes, numSlabs);
	//change default values if necessary
	void * heap= mallocHeap(heapSpace, slabBytes);
	//allocate simulated heap and store pointer to in heapStart
	FreeListNode * fln = init(heap, heapSpace, slabBytes, numSlabs, readySlabs);
	//prints malloc dividing part of header and divides up heap into initial number of slabs
	vector <FreeListNode *> fl;
	fl.push_back(fln);
	freelist(heap, readySlabs, fl);
	//prints freelist part of initial header
	cout << "Entering main loop:" << endl;
	userDriver(heap, fl, heapSpace, slabBytes, numSlabs, readySlabs, takenSlabs, mallocedMem);
	//cin user inputs until they enter quit

	return 0;
}
//------------------------------------------------------------------
void checkCLArgs(vector<string> commandLine, int &z, int &s, int &c)
{
	if (commandLine.empty())
	{
		return;
	}
	else if(commandLine[0] == "z")
	{
		z = stoi(commandLine[1]);
		//check for modified heap size
	}
	else if(commandLine[0] == "s")
	{
		s = stoi(commandLine[1]);
		//check for modified size of slab unit
	}
	else if(commandLine[0] == "c")
	{
		c = stoi(commandLine[1]);
		//check for modified number of slabs allocated at one time
	}
	else
	{
		cout << "Unrecognized Command Line Argument" << endl;
		exit(-1);
	} 
}
//------------------------------------------------------------------
void * mallocHeap(int z, int s)
{
	int numToPow = z/16;
	int bytes = pow(16, numToPow);
	//get actual number of bytes
	void * heap = malloc(bytes);
	//use actual malloc to allocate simulated heap
	return(heap);
	//return pointer to start of heap
}
//------------------------------------------------------------------
FreeListNode * init(void * heap, int z, int s, int c, vector <void *> &readySlabs)
{
	void * currSlab = heap;

	cout << "Mode: ff" << endl;
	cout << "Heap (KB): " << z << endl;
	cout << "Slab Size (B): " << s << endl;
	cout << "Slabs Alloced At One Time: " << c << endl;
	int numToPow = z/16;
	int bytes = pow(16, numToPow);
	//acutal bytes allocated
	cout << "Heap Initialized With: " << bytes << " bytes" << endl;
	int slabSize = roundUp(s);
	//slab size + MallocHeader rounded up to multiple of 32

	FreeListNode * fln = mallocDivide(c, currSlab, slabSize, bytes, heap, readySlabs);

	return fln;
}
//------------------------------------------------------------------
FreeListNode * mallocDivide(int numSlabs, void * ptr, int slabSize, int &bytes, void * heap, vector <void *> &readySlabs)
{
	for(int i = 0; i < numSlabs; i++)
	{
		cout << "Malloc dividing: " << dec << bytes << " at: "; 
		cout << printAddress(ptr, heap);
		cout << " into: "<< dec << slabSize << " and: " << (bytes-slabSize) << endl;
		//currSlab = ((char *)currSlab + 12);
		ptr = myMalloc(ptr, slabSize);
		//allocate free slab at current address, store ptr to mallocheader in currSlab
		cout << "Malloc returning: " << printAddress(ptr, heap) << endl;
		bytes -= slabSize;
		readySlabs.push_back(ptr);
		//push back actual address into readySlabs vector
		ptr = ((char *)ptr + (slabSize - 12));
	}

	FreeListNode * fln = new (ptr) FreeListNode();
	fln->size = bytes;
	fln->next = nullptr;
	fln->MAGIC_1 = 0xCCC0;
	fln->MAGIC_2 = 0xCCC0;

	return fln;
}
//------------------------------------------------------------------
void userDriver(void * heap, vector <FreeListNode *> fl, int z, int s, int c, vector <void *> readySlabs, vector <void *> takenSlabs, vector <void *> mallocedMem)
{
	string userIn = "";

	int slabSize = roundUp(s);
	
	while(userIn != "quit")
	{
		getline(cin, userIn);

		int firstComma = userIn.find(",");

		if(userIn[0] == '#')
		{
			continue;
		}
		else if(userIn == "slaballoc")
		{
			int slabSize = roundUp(s);
			slaballoc(heap, slabSize, fl, readySlabs, takenSlabs, c, fl.back()->size);
		}
		else if(userIn == "freelist")
		{
			freelist(heap, readySlabs, fl);
		}
		else if(userIn.substr(0, firstComma) == "slabfree")
		{
			string address = userIn.substr(firstComma+1);
			slabfree(address, takenSlabs, readySlabs, heap);
		}
		else if(userIn.substr(0,firstComma) == "free")
		{
			string address = userIn.substr(firstComma+1);
			free(address, takenSlabs, readySlabs, heap, fl);
		}
		else if(userIn.substr(0, firstComma) == "malloc")
		{
			int size = stoi(userIn.substr(firstComma+1));
			userMalloc(size, fl, mallocedMem);
		}
		else if(userIn.substr(0, firstComma) == "probe")
		{
			string address = userIn.substr(firstComma+1);
			probe(address, takenSlabs, readySlabs, fl, heap);
		}
		else if(userIn.substr(0, firstComma) == "read")
		{
			string address = userIn.substr(firstComma+1);
			read(address, takenSlabs, readySlabs, fl, heap);
		}
		else if(userIn.substr(0, firstComma) == "write")
		{
			string afterCommand = userIn.substr(firstComma+1);
			int secondComma = afterCommand.find(",");
			string address = afterCommand.substr(0,secondComma);
			string sVal = afterCommand.substr(secondComma+1);
			uint32_t val = (uint32_t)stoul(sVal,0,16);
			write(address, val, takenSlabs, readySlabs, fl, heap);
		}
		else
		{
			if(userIn != "quit")
			{
				cout << "Unrecognized Input" << endl;
			}
		}
	}
}
//------------------------------------------------------------------
int roundUp(int numToRound)
{
	numToRound+=12;
	//account for malloc header

    int remainder = numToRound % 32;
    if (remainder == 0)
	{
        return numToRound;
	}

    return numToRound + 32 - remainder;
}
//------------------------------------------------------------------
void * myMalloc(void * ptr, int slabSize)
{
	MallocHeader * m = new (ptr) MallocHeader();
	m->MAGIC_1 = 0xCCC0;
	m->size = slabSize;
	m->MAGIC_2 = 0xCCC0;

	void * slabPtr = (void *)((char *)ptr + 12);
	void * slab = new (slabPtr) void*();

	return slabPtr;
	//return start of slab
}
//------------------------------------------------------------------
void freelist(void * heap, vector <void *> readySlabs, vector <FreeListNode *> fl)
{
	cout << "Free slabs: " << endl;
	bool empty = false;
	int sz = readySlabs.size();
	int i = 0;
	//print slabs from zeroBased vector, 8 per line
	if(sz == 0)
	{
		cout << "Empty" << endl;
		return;
	}
	while(!empty)
	{
		
		if((i % 7 == 0 && i != 0) || sz == 1 || i == (sz-1))
		{
			cout << printAddress(readySlabs[i], heap);
			void * mHeadPtr = (void *)((char *)readySlabs[i] - 12);
			cout << magicCheck(mHeadPtr) << endl;
		}
		else
		{
			cout << printAddress(readySlabs[i], heap);
			void * mHeadPtr = (void *)((char *)readySlabs[i] - 12);
			cout << magicCheck(mHeadPtr) << ", ";
		}
		i++;
		if(i == sz)
		{
			empty = true;
		}
	}
	empty = false;
	sz = fl.size();
	i = 0;
	cout << "Free memory: " << endl;
	//print all free blocks of memory from free list vector
	while(!empty)
	{
		if((i % 1 == 0 && i != 0) || sz == 1)
		{
			cout << printAddress(fl[i], heap) << " (" << dec << fl[i]->size << ")" << "(" << printAddress(fl[i]->next, heap) << ")";
			cout << magicCheck(fl[i]) << endl;
		}
		else
		{
			cout << printAddress(fl[i], heap) << " (" << dec << fl[i]->size << ")" << "(" << printAddress(fl[i]->next, heap) << ")";
			cout << magicCheck(fl[i]) << ", ";
		}
		i++;
		if(i == sz)
		{
			empty = true;
		}
	}
	cout << "There are: " << dec << fl.size() << " free blocks." << endl;
	cout << "Largest free block: " << largestFreeBlock(fl) << endl;
	cout << "Smallest free block: " << smallestFreeBlock(fl) << endl;
}
//------------------------------------------------------------------
string printAddress(void * address, void * heap) 
{
	if (address == nullptr) 
	{
		return "nullptr";
	}
	stringstream ss;
	ss << "0x" << hex << setfill('0') << setw(8) << (char *) address - (char *) heap;
	return ss.str();
}
//------------------------------------------------------------------
int largestFreeBlock(vector <FreeListNode *> fl)
{
	int lrg = fl[0]->size;

	for(int i = 1; i < fl.size(); i++)
	{
		if(fl[i]->size > lrg)
		{
			lrg = fl[i]->size;
		}
	}
	return lrg;
}
//------------------------------------------------------------------
int smallestFreeBlock(vector <FreeListNode *> fl)
{
	int sml = fl[0]->size;

	for(int i = 1; i < fl.size(); i++)
	{
		if(fl[i]->size < sml)
		{
			sml = fl[i]->size;
		}
	}
	return sml;
}
//------------------------------------------------------------------
string magicCheck(void * ptr)
{
	int magic1 = *((int*)(ptr));
	ptr = (void*)((char*)ptr + 8);
	int magic2 = *((int*)(ptr));
	ptr = (void*)((char*)ptr + 4);
	int flMagic2 = *((int*)(ptr));

	if(magic1 == 52416 && (magic2 == 52416 || flMagic2 == 52416))
	{
		return "+";
	}
	else
	{
		return "-";
	}
}
//------------------------------------------------------------------
void slaballoc(void * heap, int slabSize, vector <FreeListNode *> &fl, vector <void *> &readySlabs, vector <void *> &takenSlabs, int numSlabs, int bytes)
{
	if(readySlabs.empty())
	{
		FreeListNode * fln = mallocDivide(numSlabs, fl.back(), slabSize, bytes, heap, readySlabs);
		fl.pop_back();
		fl.push_back(fln);
		slaballoc(heap, slabSize, fl, readySlabs, takenSlabs, numSlabs, bytes);
	}
	else
	{
		void * slab = readySlabs.back();
		readySlabs.pop_back();
		cout << "Allocated a slab at: " << printAddress(slab, heap) << endl;
		takenSlabs.push_back(slab);
	}
}
//------------------------------------------------------------------
void slabfree(string address, vector <void *> &takenSlabs, vector <void *> &readySlabs, void * heap)
{
	if(address == "" || address == "slabfree")
	{
		cout << "Error: missing addrees" << endl;
		return;
	}

	stringstream ss;
	ss << "0x" << hex << setfill('0') << setw(8) << address;

	for(int i = 0; i < takenSlabs.size(); i++)
	{
		if(ss.str() == printAddress(takenSlabs[i], heap))
		{
			void * slab = takenSlabs[i];
			string check = magicCheck((void*)((char *)slab - 12));
			if(check == "+")
			{
				takenSlabs.erase(takenSlabs.begin()+i);
				readySlabs.push_back(slab);
				return;
			}
			else
			{
				cout << "Error: memory at " << printAddress(slab, heap) << " is corrupt or not a MallocHeader" << endl;
				return;
			}
		}
	}
	cout << "Error: address outside heap" << endl;
}
//------------------------------------------------------------------
void free(string address, vector <void *> &takenSlabs, vector <void *> &readySlabs, void * heap, vector <FreeListNode *> &fl)
{
	if(address == "" || address == "free")
	{
		cout << "Error: missing addrees" << endl;
		return;
	}

	stringstream ss;
	ss << "0x" << hex << setfill('0') << setw(8) << address;

	//Check takenSlabs
	for(int i = 0; i < takenSlabs.size(); i++)
	{
		if(ss.str() == printAddress(takenSlabs[i], heap))
		{
			void * slab = takenSlabs[i];
			void * slabheader = (void *)((char *)slab - 12);
			string check = magicCheck(slabheader);
			if(check == "+")
			{
				MallocHeader * m = (MallocHeader *)slabheader;
				int sz = m->size;
				cout << "Free delinked unallocated slab" << endl;
				cout << "Free replace head_ptr with: " << printAddress(slabheader, heap) << " with size: " << sz << endl;
				FreeListNode * fln = new (slabheader) FreeListNode();
				fln->MAGIC_1 = 0xCCC0;
				fln->size = sz;
				fln->MAGIC_2 = 0xCCC0;
				fln->next = fl[0];
				fl.insert(fl.begin(),fln);
				takenSlabs.erase(takenSlabs.begin()+i);
				return;
			}
			else
			{
				cout << "Error: memory at " << printAddress(slab, heap) << " is corrupt or not a MallocHeader" << endl;
				return;
			}
		}
	}
	//Check readySlabs
	for(int i = 0; i < readySlabs.size(); i++)
	{
		if(ss.str() == printAddress(readySlabs[i], heap))
		{
			void * slab = readySlabs[i];
			string check = magicCheck((void*)((char *)slab - 12));
			if(check == "+")
			{
				void * slabheader = (void *)((char *)slab - 12);
				MallocHeader * m = (MallocHeader *)slabheader;
				int sz = m->size;
				cout << "Free delinked unallocated slab" << endl;
				cout << "Free replace head_ptr with: " << printAddress(slabheader, heap) << " with size: " << sz << endl;
				FreeListNode * fln = new (slabheader) FreeListNode();
				fln->MAGIC_1 = 0xCCC0;
				fln->size = sz;
				fln->MAGIC_2 = 0xCCC0;
				fln->next = fl[0];
				fl.insert(fl.begin(),fln);
				readySlabs.erase(readySlabs.begin()+i);
				return;
			}
			else
			{
				cout << "Error: memory at " << printAddress(slab, heap) << " is corrupt or not a MallocHeader" << endl;
				return;
			}
		}
	}
	cout << "Error: address outside heap" << endl;
}
//------------------------------------------------------------------
void probe(string address, vector <void *> takenSlabs, vector <void *> readySlabs, vector <FreeListNode *> fl, void * heap)
{
	if(address == "" || address == "probe")
	{
		cout << "Error: missing addrees" << endl;
		return;
	}

	stringstream ss;
	ss << "0x" << hex << setfill('0') << setw(8) << address;

	//check free slabs
	for(int i = 0; i < readySlabs.size(); i++)
	{
		char * currPtr = (char *)readySlabs[i] - 12;
		MallocHeader * m  = (MallocHeader *) (void *)currPtr;
		int size = m->size;

		for(int j = 0; j < size; j++)
		{
			void * adr = (void *)(currPtr + j);
			if(ss.str() == printAddress(adr,heap))
			{
				cout << "Address: " << ss.str() << " is located in a free slab" << endl;
				return;
			}
		}
	}
	//check allocated slabs
	for(int i = 0; i < takenSlabs.size(); i++)
	{
		char * currPtr = (char *)takenSlabs[i] - 12;
		MallocHeader * m  = (MallocHeader *) (void *)currPtr;
		int size = m->size;

		for(int j = 0; j < size; j++)
		{
			void * adr = (void *)(currPtr + j);
			if(ss.str() == printAddress(adr,heap))
			{
				cout << "Address: " << ss.str() << " is located in an allocated slab" << endl;
				return;
			}
		}
	}
	//check free list
	for(int i = 0; i <= fl.size(); i++)
	{
		char * currPtr = (char *)fl[i];
		FreeListNode * fl = (FreeListNode *)(void *)currPtr;
		int size = fl->size;
		for(int j = 0; j < size; j++)
		{
			void * adr = (void *)(currPtr + j);
			if(ss.str() == printAddress(adr, heap))
			{
				cout << "Address: " << ss.str() << " is located in a free block" << endl;
				return;
			}
		}
	}
	cout << "Error: address outside heap" << endl;
}
//------------------------------------------------------------------
void read(string address, vector <void *> takenSlabs, vector <void *> readySlabs, vector <FreeListNode *> fl, void * heap)
{
	if(address == "" || address == "read")
	{
		cout << "Error: missing addrees" << endl;
		return;
	}

	stringstream ss;
	ss << "0x" << hex << setfill('0') << setw(8) << address;

	//check free slabs
	for(int i = 0; i < readySlabs.size(); i++)
	{
		char * currPtr = (char *)readySlabs[i] - 12;
		MallocHeader * m  = (MallocHeader *) (void *)currPtr;
		int size = m->size;

		for(int j = 0; j < size; j++)
		{
			void * adr = (void *)(currPtr + j);
			if(ss.str() == printAddress(adr,heap))
			{
				uint32_t ret = (*(uint32_t *)(adr));
				cout << "Address: " << ss.str() << " contains (uint32_t): 0x" <<  hex << ret << endl;
				return;
			}
		}
	}
	//check allocated slabs
	for(int i = 0; i < takenSlabs.size(); i++)
	{
		char * currPtr = (char *)takenSlabs[i] - 12;
		MallocHeader * m  = (MallocHeader *) (void *)currPtr;
		int size = m->size;

		for(int j = 0; j < size; j++)
		{
			void * adr = (void *)(currPtr + j);
			if(ss.str() == printAddress(adr,heap))
			{
				uint32_t ret = (*(uint32_t *)(adr));
				cout << "Address: " << ss.str() << " contains (uint32_t): 0x" <<  hex << ret << endl;
				return;
			}
		}
	}
	//check free list
	for(int i = 0; i <= fl.size()-1; i++)
	{
		char * currPtr = (char *)fl[i];
		FreeListNode * fl = (FreeListNode *)(void *)currPtr;
		int size = fl->size;
		for(int j = 0; j < size; j++)
		{
			void * adr = (void *)(currPtr + j);
			if(ss.str() == printAddress(adr, heap))
			{
				uint32_t ret = (*(uint32_t *)(adr));
				cout << "Address: " << ss.str() << " contains (uint32_t): 0x" <<  hex << ret << endl;
				return;
			}
		}
	}
	cout << "Error: address outside heap" << endl;
}
//------------------------------------------------------------------
void write(string address, uint32_t value, vector <void *> takenSlabs, vector <void *> readySlabs, vector <FreeListNode *> fl, void * heap)
{
	if(address == "" || address == "read")
	{
		cout << "Error: missing addrees" << endl;
		return;
	}


	stringstream ss;
	ss << "0x" << hex << setfill('0') << setw(8) << address;

	//check free slabs
	for(int i = 0; i < readySlabs.size(); i++)
	{
		char * currPtr = (char *)readySlabs[i] - 12;
		MallocHeader * m  = (MallocHeader *) (void *)currPtr;
		int size = m->size;

		for(int j = 0; j < size; j++)
		{
			void * adr = (void *)(currPtr + j);
			if(ss.str() == printAddress(adr,heap))
			{
				uint32_t * newVal = new (adr) uint32_t(value);
				cout << "Address: " << ss.str() << " set to (uint32_t): 0x" << hex << value << endl;
				return;
			}
		}
	}
	//check allocated slabs
	for(int i = 0; i < takenSlabs.size(); i++)
	{
		char * currPtr = (char *)takenSlabs[i] - 12;
		MallocHeader * m  = (MallocHeader *) (void *)currPtr;
		int size = m->size;

		for(int j = 0; j < size; j++)
		{
			void * adr = (void *)(currPtr + j);
			if(ss.str() == printAddress(adr,heap))
			{
				uint32_t * newVal = new (adr) uint32_t(value);
				cout << "Address: " << ss.str() << " set to (uint32_t): 0x" << hex << value << endl;
				return;
			}
		}
	}
	//check free list
	for(int i = 0; i < fl.size(); i++)
	{
		char * currPtr = (char *)fl[i];
		FreeListNode * fl = (FreeListNode *)(void *)currPtr;
		int size = fl->size;
		for(int j = 0; j < size; j++)
		{
			void * adr = (void *)(currPtr + j);
			if(ss.str() == printAddress(adr, heap))
			{
				uint32_t * newVal = new (adr) uint32_t(value);
				cout << "Address: " << ss.str() << " set to (uint32_t): 0x" << hex << value << endl;
				return;
			}
		}
	}
	cout << "Error: address outside heap" << endl;
}
//------------------------------------------------------------------
void userMalloc(int size, vector <FreeListNode *> &fl, vector <void *> &mallocedMem)
{
	for(int i = 0; i < fl.size(); i++)
	{
		char * currPtr = (char *)fl[i];
		FreeListNode * fln = (FreeListNode *)(void *)currPtr;
		int nodeSize = fln->size;

		if(size < nodeSize)
		{
			void * ptr = new (currPtr) void*();
			mallocedMem.push_back(ptr);
			void * newLoc = (void *)((char *)ptr + size);
			FreeListNode * newFln = new (newLoc) FreeListNode();
			newFln->size = nodeSize - size;
			if(i == fl.size()-1)
			{
				newFln->next = nullptr;
			}
			else
			{
				newFln->next = fl[i+1];	
			}
			newFln->MAGIC_1 = 0xCCC0;
			newFln->MAGIC_2 = 0xCCC0;
			fl[i] = newFln;
		}
		else if(size == nodeSize)
		{
			void * ptr = new (currPtr) void*();
			mallocedMem.push_back(ptr);
			fl.erase(fl.begin()+i);
		}
	}
}
//------------------------------------------------------------------