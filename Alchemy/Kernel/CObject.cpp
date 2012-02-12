//	CObject.cpp
//
//	Methods for basic objects and class objects

#include "Kernel.h"
#include "KernelObjID.h"

//	For now we hard-code the maximum number of object classes

#define MAX_OBJCLASS_COUNT							1000

//	Array to hold all object classes

typedef IObjectClass *ClassArray[MAX_OBJCLASS_COUNT];
static ClassArray g_Classes[OBJCLASS_MODULE_COUNT];

//	CObject methods

CObject::CObject (IObjectClass *pClass) : m_pClass(pClass)

//	CObject constructor

	{
	}

CObject::~CObject (void)

//	CObject destructor

	{
	}

CObject *CObject::Copy (void)

//	Copy
//
//	Creates a copy of the object and returns it

	{
	CObject *pCopy;
	BYTE *pSource;
	BYTE *pDest;
	PDATADESCSTRUCT pPos;

	//	Create a new copy

	pCopy = Clone();

	//	Start traversing

	pPos = DataDescStart();
	VerifyDataDesc();
	pSource = DataStart();
	pDest = pCopy->DataStart();

	if (!CopyData(pPos, &pSource, &pDest))
		return NULL;

	//	After we're done, call the copy handler in case the
	//	object wants to do anything.

	pCopy->CopyHandler(this);

	return pCopy;
	}

BOOL CObject::CopyData (PDATADESCSTRUCT pPos, BYTE **iopSource, BYTE **iopDest)

//	CopyData
//
//	Copies the data from source to destination using the given data descriptor
//	table. Returns the updated positions

	{
	BYTE *pSource = *iopSource;
	BYTE *pDest = *iopDest;
	int iAllocSize = -1;

	while (pPos)
		{
		switch (pPos->iOpCode)
			{
			case DATADESC_OPCODE_INT:
			case DATADESC_OPCODE_REFERENCE:
			case DATADESC_OPCODE_ALLOC_SIZE32:
				{
				int i;

				//	If this is an alloc size op code, then remember
				//	the allocated size

				if (pPos->iOpCode == DATADESC_OPCODE_ALLOC_SIZE32)
					iAllocSize = sizeof(DWORD) * (*((int *)pSource));

				//	Copy integers over

				for (i = 0; i < pPos->iCount; i++)
					{
					*((DWORD *)pDest) = *((DWORD *)pSource);
					pDest += sizeof(int);
					pSource += sizeof(int);
					}
				break;
				}

			case DATADESC_OPCODE_ALLOC_MEMORY:
				{
				int i;
				BYTE *pSourceMem = (BYTE *)*((DWORD *)pSource);
				BYTE *pDestMem;

				//	Allocate a block of memory

				if (pSourceMem)
					{
					ASSERT(iAllocSize != -1);
					pDestMem = (BYTE *)MemAlloc(iAllocSize);
					if (pDestMem == NULL)
						goto Fail;

					*((DWORD *)pDest) = (DWORD)pDestMem;

					for (i = 0; i < iAllocSize; i++)
						*pDestMem++ = *pSourceMem++;
					}
				else
					*((DWORD *)pDest) = 0;

				pSource += sizeof(LPVOID);
				pDest += sizeof(LPVOID);
				iAllocSize = -1;
				break;
				}

			case DATADESC_OPCODE_ALLOC_OBJ:
				{
				CObject *pObj;
				CObject *pCopy;

				//	Cast the source to an object

				pObj = *(CObject **)pSource;

				//	Make a copy of the object and store it in the
				//	destination

				if (pObj)
					{
					ASSERT(*((DWORD *)pDest) == 0);
					pCopy = pObj->Copy();
					if (pCopy == NULL)
						return NULL;

					*((DWORD *)pDest) = (DWORD)pCopy;
					}
				else
					*((DWORD *)pDest) = 0;

				//	Next

				pSource += sizeof(LPVOID);
				pDest += sizeof(LPVOID);

				break;
				}

			case DATADESC_OPCODE_EMBED_OBJ:
				{
				PDATADESCSTRUCT pObjPos;
				CObject *pObj;
				CObject *pObjDest;

				//	Cast the source to an object and get the data description

				pObj = (CObject *)pSource;
				pObjDest = (CObject *)pDest;
				pObjPos = pObj->DataDescStart();
				pObj->VerifyDataDesc();

				//	Copy the vtable and the class object pointer

				ASSERT(sizeof(CObject) == 2 * sizeof(LPVOID));
				*((DWORD *)pDest) = *((DWORD *)pSource);
				pDest += sizeof(LPVOID);
				pSource += sizeof(LPVOID);
				*((DWORD *)pDest) = *((DWORD *)pSource);
				pDest += sizeof(LPVOID);
				pSource += sizeof(LPVOID);

				//	Copy the rest of the object

				if (!CopyData(pObjPos, &pSource, &pDest))
					goto Fail;

				//	Call the copy handler

				pObjDest->CopyHandler(pObj);
				break;
				}

			case DATADESC_OPCODE_ZERO:
				{
				int i;

				pSource += pPos->iCount * sizeof(DWORD);
				for (i = 0; i < pPos->iCount; i++)
					{
					*((DWORD *)pDest) = 0;
					pDest += sizeof(int);
					}

				break;
				}

			case DATADESC_OPCODE_VTABLE:
				{
				pSource += pPos->iCount * sizeof(LPVOID);
				pDest += pPos->iCount * sizeof(LPVOID);
				break;
				}

			default:
				ASSERT(FALSE);
			}

		pPos = DataDescNext(pPos);
		}

	*iopSource = pSource;
	*iopDest = pDest;
	return TRUE;

Fail:

	return FALSE;
	}

PDATADESCSTRUCT CObject::DataDescNext (PDATADESCSTRUCT pPos)

//	DataDescNext
//
//	Returns the next data descriptor record. If there are no more, it
//	returns NULL

	{
	//	Skip

	pPos++;

	//	If this is the end record, then we're done

	if (pPos->iOpCode == DATADESC_OPCODE_STOP)
		return NULL;

	//	Otherwise, we return the record

	return pPos;
	}

PDATADESCSTRUCT CObject::DataDescStart (void)

//	DataDescStart
//
//	Returns the first data descriptor record

	{
	return GetClass()->GetDataDesc();
	}

BYTE *CObject::DataStart (void)

//	DataStart
//
//	Returns a pointer to the start of member data for this
//	object.

	{
	BYTE *pData = (BYTE *)this;

	//	Skip past the CObject class

	pData += sizeof(CObject);

	return pData;
	}

ALERROR CObject::Load (CUnarchiver *pUnarchiver)

//	Load
//
//	Loads object from an archive

	{
	return LoadHandler(pUnarchiver);
	}

ALERROR CObject::LoadDone (void)

//	LoadDone
//
//	This method is called when an unarchiver is done loading an object.

	{
	ALERROR error;
	PDATADESCSTRUCT pPos;
	BYTE *pDest;

	//	Let all our sub-objects know that we're done

	pPos = DataDescStart();
	VerifyDataDesc();
	pDest = DataStart();

	while (pPos)
		{
		switch (pPos->iOpCode)
			{
			case DATADESC_OPCODE_INT:
			case DATADESC_OPCODE_ALLOC_SIZE32:
			case DATADESC_OPCODE_REFERENCE:
			case DATADESC_OPCODE_ZERO:
			case DATADESC_OPCODE_VTABLE:
				pDest += pPos->iCount * sizeof(LPVOID);
				break;

			case DATADESC_OPCODE_ALLOC_MEMORY:
				pDest += sizeof(LPVOID);
				break;

			case DATADESC_OPCODE_ALLOC_OBJ:
				{
				CObject *pObj = *(CObject **)pDest;

				if (pObj)
					if (error = pObj->LoadDone())
						return error;

				pDest += sizeof(LPVOID);
				break;
				}

			case DATADESC_OPCODE_EMBED_OBJ:
				{
				CObject *pObj;

				pObj = (CObject *)pDest;
				if (error = pObj->LoadDone())
					return error;

				//	Advance the pointer by the size of the object

				pDest += pObj->m_pClass->GetObjSize();
				break;
				}

			default:
				ASSERT(FALSE);
			}

		pPos = DataDescNext(pPos);
		}

	//	Let this object know that it is done

	if (error = LoadDoneHandler())
		return error;

	return NOERROR;
	}

ALERROR CObject::LoadHandler (CUnarchiver *pUnarchiver)

//	LoadHandler
//
//	Handles loading the actual object data

	{
	ALERROR error;
	PDATADESCSTRUCT pPos;
	BYTE *pDest;
	int iAllocSize = -1;

	//	Write out the object data

	pPos = DataDescStart();
	VerifyDataDesc();
	pDest = DataStart();

	while (pPos)
		{
		switch (pPos->iOpCode)
			{
			case DATADESC_OPCODE_REFERENCE:
				{
				int i;

				for (i = 0; i < pPos->iCount; i++)
					{
					int iID;

					//	Read ID

					if (error = pUnarchiver->ReadData((char *)&iID, sizeof(int)))
						goto Fail;

					if (error = pUnarchiver->ResolveReference(iID, (void **)pDest))
						goto Fail;

					pDest += sizeof(LPVOID);
					}

				break;
				}

			case DATADESC_OPCODE_INT:
			case DATADESC_OPCODE_ALLOC_SIZE32:
				{
				//	Read the integers

				if (error = pUnarchiver->ReadData((char *)pDest, sizeof(int) * pPos->iCount))
					goto Fail;

				//	If this is an alloc size op code, then remember
				//	the allocated size

				if (pPos->iOpCode == DATADESC_OPCODE_ALLOC_SIZE32)
					iAllocSize = sizeof(DWORD) * (*((int *)pDest));

				pDest += sizeof(int) * pPos->iCount;
				break;
				}

			case DATADESC_OPCODE_ALLOC_MEMORY:
				{
				char *pBlock;

				//	Allocate a block of memory

				ASSERT(iAllocSize != -1);
				pBlock = (char *)MemAlloc(iAllocSize);
				if (pBlock == NULL)
					{
					error = ERR_MEMORY;
					goto Fail;
					}

				*((DWORD *)pDest) = (DWORD)pBlock;

				//	Read the block

				if (error = pUnarchiver->ReadData((char *)pBlock, iAllocSize))
					goto Fail;

				pDest += sizeof(LPVOID);
				iAllocSize = -1;
				break;
				}

			case DATADESC_OPCODE_ALLOC_OBJ:
				{
				CObject *pObj;
				IObjectClass *pClass;
				OBJCLASSID ObjID;

				//	Create an object of the class

				if (error = pUnarchiver->ReadData((char *)&ObjID, sizeof(ObjID)))
					goto Fail;

				if (ObjID)
					{
					pClass = CObjectClassFactory::GetClass(ObjID);
					if (pClass == NULL)
						{
						error = ERR_CLASSNOTFOUND;
						goto Fail;
						}

					pObj = pClass->Instantiate();
					if (pObj == NULL)
						{
						error = ERR_FAIL;
						goto Fail;
						}

					*((LPVOID *)pDest) = (LPVOID)pObj;

					//	Load the object

					if (error = pObj->Load(pUnarchiver))
						goto Fail;
					}
				else
					*((LPVOID *)pDest) = NULL;

				//	Next

				pDest += sizeof(LPVOID);
				break;
				}

			case DATADESC_OPCODE_EMBED_OBJ:
				{
				CObject *pObj;
				OBJCLASSID ObjID;

				pObj = (CObject *)pDest;

				if (pPos->dwFlags & DATADESC_FLAG_CUSTOM)
					{
					if (error = LoadCustom(pUnarchiver, pDest))
						goto Fail;
					}
				else
					{
					//	Load the object ID

					if (error = pUnarchiver->ReadData((char *)&ObjID, sizeof(ObjID)))
						goto Fail;

					//	Load the object

					if (ObjID != pObj->GetClass()->GetObjID())
						goto Fail;

					if (error = pObj->Load(pUnarchiver))
						goto Fail;
					}

				//	Advance the pointer by the size of the object

				pDest += pObj->m_pClass->GetObjSize();
				break;
				}

			case DATADESC_OPCODE_ZERO:
			case DATADESC_OPCODE_VTABLE:
				{
				//	Skip this

				pDest += sizeof(DWORD) * pPos->iCount;
				break;
				}

			default:
				ASSERT(FALSE);
			}

		pPos = DataDescNext(pPos);
		}

	return NOERROR;

Fail:

	return error;
	}

ALERROR CObject::Save (CArchiver *pArchiver)

//	Save
//
//	Saves object to an archive

	{
	ALERROR error;
	DWORD dwID;

	//	Write out the ID of object class

	dwID = m_pClass->GetObjID();
	if (error = pArchiver->WriteData((char *)&dwID, sizeof(DWORD)))
		return error;

	if (error = SaveHandler(pArchiver))
		return error;

	return NOERROR;
	}

ALERROR CObject::SaveHandler (CArchiver *pArchiver)

//	SaveHandler
//
//	Saves the object data

	{
	ALERROR error;
	PDATADESCSTRUCT pPos;
	BYTE *pSource;
	int iAllocSize = -1;

	//	Write out the object data

	pPos = DataDescStart();
	VerifyDataDesc();
	pSource = DataStart();

	while (pPos)
		{
		switch (pPos->iOpCode)
			{
			case DATADESC_OPCODE_REFERENCE:
				{
				int i;

				for (i = 0; i < pPos->iCount; i++)
					{
					int iID;

					if (error = pArchiver->Reference2ID(*(void **)pSource, &iID))
						goto Fail;

					if (error = pArchiver->WriteData((char *)&iID, sizeof(int)))
						goto Fail;

					pSource += sizeof(LPVOID);
					}

				break;
				}

			case DATADESC_OPCODE_INT:
			case DATADESC_OPCODE_ALLOC_SIZE32:
				{
				//	If this is an alloc size op code, then remember
				//	the allocated size

				if (pPos->iOpCode == DATADESC_OPCODE_ALLOC_SIZE32)
					iAllocSize = sizeof(DWORD) * (*((int *)pSource));

				//	Write out the integers

				if (error = pArchiver->WriteData((char *)pSource, sizeof(int) * pPos->iCount))
					goto Fail;

				pSource += sizeof(int) * pPos->iCount;
				break;
				}

			case DATADESC_OPCODE_ALLOC_MEMORY:
				{
				BYTE *pSourceMem = (BYTE *)*((DWORD *)pSource);

				//	Write out the block

				ASSERT(iAllocSize != -1);
				if (error = pArchiver->WriteData((char *)pSourceMem, iAllocSize))
					goto Fail;

				pSource += sizeof(LPVOID);
				iAllocSize = -1;
				break;
				}

			case DATADESC_OPCODE_ALLOC_OBJ:
				{
				CObject *pObj;

				//	Cast the source to an object

				pObj = *(CObject **)pSource;

				//	If the field is NULL, then just save out Zero

				if (pObj == NULL)
					{
					DWORD dwZero = 0;

					if (error = pArchiver->WriteData((char *)&dwZero, sizeof(DWORD)))
						goto Fail;
					}
				else
					{
					//	Store the object in line

					if (error = pObj->Save(pArchiver))
						goto Fail;
					}

				//	Next

				pSource += sizeof(LPVOID);
				break;
				}

			case DATADESC_OPCODE_EMBED_OBJ:
				{
				CObject *pObj;

				//	Cast the source to an object

				pObj = (CObject *)pSource;

				//	Store the object in line

				if (pPos->dwFlags & DATADESC_FLAG_CUSTOM)
					{
					if (error = SaveCustom(pArchiver, pSource))
						goto Fail;
					}
				else
					{
					if (error = pObj->Save(pArchiver))
						goto Fail;
					}

				//	Advance the pointer by the size of the object

				pSource += pObj->m_pClass->GetObjSize();
				break;
				}

			case DATADESC_OPCODE_ZERO:
			case DATADESC_OPCODE_VTABLE:
				{
				//	Skip this

				pSource += sizeof(DWORD) * pPos->iCount;
				break;
				}

			default:
				ASSERT(FALSE);
			}

		pPos = DataDescNext(pPos);
		}

	return NOERROR;

Fail:

	return error;
	}

void CObject::VerifyDataDesc (void)

//	VerifyDataDesc
//
//	Verify that the data desc matches the object
//	ASSERTs if it does not

	{
	//	Verify that the data descriptor table matches
	//	the actual structure

#ifdef _DEBUG
	PDATADESCSTRUCT pPos = m_pClass->GetDataDesc();
	int iTotalSize = sizeof(CObject);

	if (pPos)
		{
		while (pPos)
			{
			switch (pPos->iOpCode)
				{
				case DATADESC_OPCODE_INT:
				case DATADESC_OPCODE_REFERENCE:
				case DATADESC_OPCODE_ZERO:
				case DATADESC_OPCODE_VTABLE:
					iTotalSize += sizeof(int) * pPos->iCount;
					break;

				case DATADESC_OPCODE_ALLOC_MEMORY:
				case DATADESC_OPCODE_ALLOC_SIZE32:
				case DATADESC_OPCODE_ALLOC_OBJ:
					iTotalSize += sizeof(int);
					break;

				case DATADESC_OPCODE_EMBED_OBJ:
					{
					CObject *pObj;
					pObj = (CObject *)(((BYTE *)this) + iTotalSize);
					iTotalSize += pObj->m_pClass->GetObjSize();
					break;
					}

				default:
					ASSERT(FALSE);
				}

			pPos = DataDescNext(pPos);
			}

		ASSERT(iTotalSize == m_pClass->GetObjSize());
		}
#endif
	}

//	CObjectClassFactory methods

CObject *CObjectClassFactory::Create (OBJCLASSID ObjID)
	{
	IObjectClass *pClass = GetClass(ObjID);
	return pClass->Instantiate();
	}

IObjectClass *CObjectClassFactory::GetClass (OBJCLASSID ObjID)

//	GetClass
//
//	Returns a pointer to the class for the given object

	{
	int iModule = OBJCLASSIDGetModule(ObjID);
	int iID = OBJCLASSIDGetID(ObjID);

	ASSERT(iModule >= 0 && iModule < OBJCLASS_MODULE_COUNT);
	ASSERT(iID >= 0 && iID < MAX_OBJCLASS_COUNT);

	return g_Classes[iModule][iID];
	}

bool CObject::IsValidPointer (CObject *pObj)

//	IsValidPointer
//
//	Returns TRUE if pObj is a valid pointer

	{
	MEMORY_BASIC_INFORMATION mi;
	VirtualQuery(pObj, &mi, sizeof(mi));
	return (mi.State == MEM_COMMIT
			&& (mi.AllocationProtect & PAGE_READWRITE));
	}

void CObjectClassFactory::NewClass (IObjectClass *pClass)
	{
	int iModule = OBJCLASSIDGetModule(pClass->GetObjID());
	int iID = OBJCLASSIDGetID(pClass->GetObjID());

	ASSERT(g_Classes[iModule][iID] == NULL);
	g_Classes[iModule][iID] = pClass;
	}

//	Flatten methods

ALERROR CObject::Flatten (CObject *pObject, CString *retsData)

//	Flatten
//
//	Flattens the object to a memory block. This function should only be
//	used for objects that do not have references to other objects.

	{
	ALERROR error;
	CMemoryWriteStream Stream(10000000);
	CArchiver Archiver(&Stream);

	if (error = Archiver.BeginArchive())
		return error;

	if (error = Archiver.AddObject(pObject))
		return error;

	if (error = Archiver.EndArchive())
		return error;

	//	Store as a string

	*retsData = CString(Stream.GetPointer(), Stream.GetLength());

	return NOERROR;
	}

ALERROR CObject::Unflatten (CString sData, CObject **retpObject)

//	UnflattenObject
//
//	Unflattens an object flattened by FlattenObject

	{
	ALERROR error;
	CMemoryReadStream Stream(sData.GetPointer(), sData.GetLength());
	CUnarchiver Unarchiver(&Stream);

	if (error = Unarchiver.BeginUnarchive())
		return error;

	if (error = Unarchiver.EndUnarchive())
		return error;

	*retpObject = Unarchiver.GetObject(0);
	if (*retpObject == NULL)
		return ERR_FAIL;

	return NOERROR;
	}
