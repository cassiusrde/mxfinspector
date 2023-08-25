#include "stdafx.h"
#include "MXFTypes.h"
#include "MXFUtils.h"


BOOL Batch::Read(HANDLE hMXF)
{
unsigned int i;
DWORD nRead;

	num = GetNumber32(hMXF);
	size = GetNumber32(hMXF);
	ppItem = new unsigned char*[num];
	for(i = 0; i < num; i++)
	{
		ppItem[i] = new unsigned char[size];
		ReadFile(hMXF, ppItem[i], size, &nRead, NULL);
	}
	return TRUE;
}


unsigned int PartitionPack::CalcLength()
{
	length = 80 + 8 + essenceContainers.num * essenceContainers.size;

	return length;
}


unsigned int PartitionPack::Write(IWriter* pWriter)
{
DWORD totalWritten;
unsigned int i;

	CalcLength();

	totalWritten=WriteBuffer(key, 16, pWriter);
	totalWritten+=WriteBERLength(length, pWriter);
	totalWritten+=WriteNumber16(majorVersion, pWriter);
	totalWritten+=WriteNumber16(minorVersion, pWriter);
	totalWritten+=WriteNumber32(kagSize, pWriter);
	totalWritten+=WriteNumber64(thisPartition, pWriter);
	totalWritten+=WriteNumber64(previousPartition, pWriter);
	totalWritten+=WriteNumber64(footerPartition, pWriter);
	totalWritten+=WriteNumber64(headerByteCount, pWriter);
	totalWritten+=WriteNumber64(indexByteCount, pWriter);
	totalWritten+=WriteNumber32(indexSID, pWriter);
	totalWritten+=WriteNumber64(bodyOffset, pWriter);
	totalWritten+=WriteNumber32(bodySID, pWriter);
	totalWritten+=WriteBuffer(operationPattern, 16, pWriter);

	totalWritten+=WriteNumber32(essenceContainers.num, pWriter);
	totalWritten+=WriteNumber32(essenceContainers.size, pWriter);
	for(i = 0; i < essenceContainers.num; i++)
		totalWritten+=WriteBuffer(essenceContainers.ppItem[i], essenceContainers.size, pWriter);

	return totalWritten;

}


BOOL PartitionPack::Read(HANDLE hMXF, DWORD chunkSize)
{
unsigned int localTag;
unsigned int size;
DWORD nRead;
	majorVersion = GetNumber16(hMXF);
	minorVersion = GetNumber16(hMXF);
	kagSize = GetNumber32(hMXF);
	thisPartition = GetNumber64(hMXF);
	previousPartition = GetNumber64(hMXF);
	footerPartition = GetNumber64(hMXF);
	headerByteCount = GetNumber64(hMXF);
	indexByteCount = GetNumber64(hMXF);
	indexSID = GetNumber32(hMXF);
	bodyOffset = GetNumber64(hMXF);
	bodySID = GetNumber32(hMXF);
	ReadFile(hMXF, operationPattern, 16, &nRead, NULL);
	essenceContainers.num = GetNumber32(hMXF);
	essenceContainers.size = GetNumber32(hMXF);
	essenceContainers.ppItem = new unsigned char*[essenceContainers.num];
	for(unsigned int i=0; i < essenceContainers.num; i++)
	{
		essenceContainers.ppItem[i] = new unsigned char[essenceContainers.size];
		ReadFile(hMXF, essenceContainers.ppItem[i], essenceContainers.size, &nRead, NULL);
	}

	return TRUE;
}




unsigned int Preface::CalcLength()
{
	length =  16+4 +	// Instance UID 
				(bGenerationUID?(16+4):0) + // Generation UID
				8+4 + // LastModifiedDate
				2+4 + // version
				(bObjectModelVersion?(4+4):0) + // ObjectModelVersion
				(bPrimaryPackage?(16+4):0) + // PrimaryPackage
				4+8+identifications.size*identifications.num + // Identifications
				16+4 + // ContentStorage
				16+4 + // OperationalPattern
				4+8+essenceContainers.size*essenceContainers.num + // EssenceContainers
				4+8+DMSchemes.size*DMSchemes.num; // DMSchemes

	return length;
}

unsigned int Preface::Write(IWriter* pWriter)
{
DWORD totalWritten;
unsigned int i;

	CalcLength();

	totalWritten = 0;

	totalWritten = WriteBuffer(key, 16, pWriter);
	totalWritten += WriteBERLength(length, pWriter);

	totalWritten += WriteNumber16(0x3c0a, pWriter);
	totalWritten += WriteNumber16(16, pWriter);
	totalWritten += WriteBuffer(instanceUID, 16, pWriter);
	if(bGenerationUID)
	{
		totalWritten += WriteNumber16(0x0102, pWriter);
		totalWritten += WriteNumber16(16, pWriter);
		totalWritten += WriteBuffer(generationUID, 16, pWriter);
	}
	totalWritten += WriteNumber16(0x3b02, pWriter);
	totalWritten += WriteNumber16(8, pWriter);
	totalWritten += WriteNumber64(lastModifiedDate.i64Data, pWriter);

	totalWritten += WriteNumber16(0x3b05, pWriter);
	totalWritten += WriteNumber16(2, pWriter);
	totalWritten += WriteNumber16(version, pWriter);

	if(bObjectModelVersion)
	{
		totalWritten += WriteNumber16(0x3b07, pWriter);
		totalWritten += WriteNumber16(4, pWriter);
		totalWritten += WriteNumber32(objectModelVersion, pWriter);
	}

	if(bPrimaryPackage)
	{
		totalWritten += WriteNumber16(0x3b08, pWriter);
		totalWritten += WriteNumber16(16, pWriter);
		totalWritten += WriteBuffer(primaryPackage, 16, pWriter);
	}

	totalWritten += WriteNumber16(0x3b06, pWriter);
	totalWritten += WriteNumber16(8+identifications.num*identifications.size, pWriter);
	totalWritten += WriteNumber32(identifications.num, pWriter);
	totalWritten += WriteNumber32(identifications.size, pWriter);
	for(i = 0; i < identifications.num; i++)
		totalWritten += WriteBuffer(identifications.ppItem[i], identifications.size, pWriter);

	totalWritten += WriteNumber16(0x3b03, pWriter);
	totalWritten += WriteNumber16(16, pWriter);
	totalWritten += WriteBuffer(contentStorage, 16, pWriter);

	totalWritten += WriteNumber16(0x3b09, pWriter);
	totalWritten += WriteNumber16(16, pWriter);
	totalWritten += WriteBuffer(operationalPattern, 16, pWriter);

	totalWritten += WriteNumber16(0x3b0a, pWriter);
	totalWritten += WriteNumber16(8+essenceContainers.num*essenceContainers.size, pWriter);
	totalWritten += WriteNumber32(essenceContainers.num, pWriter);
	totalWritten += WriteNumber32(essenceContainers.size, pWriter);
	for(i = 0; i < essenceContainers.num; i++)
		totalWritten += WriteBuffer(essenceContainers.ppItem[i], essenceContainers.size, pWriter);

	totalWritten += WriteNumber16(0x3b0b, pWriter);
	totalWritten += WriteNumber16(8+DMSchemes.num*DMSchemes.size, pWriter);
	totalWritten += WriteNumber32(DMSchemes.num, pWriter);
	totalWritten += WriteNumber32(DMSchemes.size, pWriter);
	for(i = 0; i < DMSchemes.num; i++)
		totalWritten += WriteBuffer(DMSchemes.ppItem[i], DMSchemes.size, pWriter);
	
	return totalWritten;
}


BOOL Preface::Read(HANDLE hMXF, DWORD chunkSize)
{
unsigned int localTag;
unsigned int size;
DWORD nRead;

	while(chunkSize > 0)
	{
		localTag = GetNumber16(hMXF);
		size = GetNumber16(hMXF);
		chunkSize -= 4;
		chunkSize -= size;

		switch(localTag)
		{
			case 0x3c0a:
				ReadFile(hMXF, &instanceUID, 16, &nRead, NULL);
			break;
			case 0x0102:
				bGenerationUID = true;
				ReadFile(hMXF, &generationUID, 16, &nRead, NULL);
			break;
			case 0x3b02:
				lastModifiedDate.i64Data = GetNumber64(hMXF);
			break;
			case 0x3b05:
				version = GetNumber16(hMXF);
			break;
			case 0x3b07:
				bObjectModelVersion = true;
				objectModelVersion = GetNumber32(hMXF);
			break;
			case 0x3b08:
				bPrimaryPackage= true;
				ReadFile(hMXF, &primaryPackage, 16, &nRead, NULL);
			break;
			case 0x3b06:
				identifications.Read(hMXF);
			break;
			case 0x3b03:
				ReadFile(hMXF, &contentStorage, 16, &nRead, NULL);
			break;
			case 0x3b09:
				ReadFile(hMXF, &operationalPattern, 16, &nRead, NULL);
			break;
			case 0x3b0a:
				essenceContainers.Read(hMXF);
			break;
			case 0x3b0b:
				DMSchemes.Read(hMXF);
			break;

			default:
				SetFilePointer(hMXF, size, NULL, FILE_CURRENT);
			break;
		}

	}

	return TRUE;
}

BOOL PrimerPack::Read(HANDLE hMXF)
{
	if( localTagEntryBatch.pItems )
		delete [] localTagEntryBatch.pItems;

	unsigned int i;
	DWORD nRead;

	localTagEntryBatch.numberOfItems = GetNumber32(hMXF);
	localTagEntryBatch.itemLength = GetNumber32(hMXF);

	localTagEntryBatch.pItems = new LocalTagEntry[localTagEntryBatch.numberOfItems];
	for(i = 0; i < localTagEntryBatch.numberOfItems; i++)
	{
		localTagEntryBatch.pItems[i].localTag = GetNumber16(hMXF);
		ReadFile(hMXF, localTagEntryBatch.pItems[i].uid, 16, &nRead, NULL);
	}

	return TRUE;
}


UInt16 PrimerPack::FindLocalTag(UInt64 itemDesignator)
{
unsigned int i;
	for(i = 0; i < localTagEntryBatch.numberOfItems; i++)
	{
		//if(!memcmp(localTagEntryBatch.pItems[i].uid + 8, itemDesignator, length))
		if(itemDesignator == UInt64ByteSwap(*((UInt64*)(localTagEntryBatch.pItems[i].uid + 8))))
		{
			return localTagEntryBatch.pItems[i].localTag;
		}
	}

	return 0x0000;
}

UInt64 PrimerPack::FindItemDesignator(UInt16 localTag)
{
unsigned int i;
UInt64 itemDesignator;
	for(i = 0; i < localTagEntryBatch.numberOfItems; i++)
	{
		if(localTagEntryBatch.pItems[i].localTag == localTag)
		{
			itemDesignator = *((UInt64*)(localTagEntryBatch.pItems[i].uid + 8));
			itemDesignator = UInt64ByteSwap(itemDesignator);
			return itemDesignator;
		}
	}
	return 0;
}


unsigned int PrimerPack::CalcLength()
{
	length = 8 + localTagEntryBatch.itemLength*localTagEntryBatch.numberOfItems;

	return length;
}


unsigned int PrimerPack::Write(IWriter* pWriter)
{
DWORD totalWritten;
unsigned int i;

	CalcLength();

	totalWritten = 0;
	totalWritten+=WriteBuffer(key, 16, pWriter);
	totalWritten+=WriteBERLength(length, pWriter);

	totalWritten+=WriteNumber32(localTagEntryBatch.numberOfItems, pWriter);
	totalWritten+=WriteNumber32(localTagEntryBatch.itemLength, pWriter);
	for(i = 0; i < localTagEntryBatch.numberOfItems; i++)
	{
		totalWritten+=WriteNumber16(localTagEntryBatch.pItems[i].localTag, pWriter);
		totalWritten+=WriteBuffer(localTagEntryBatch.pItems[i].uid, 16, pWriter);
	}

	return totalWritten;
}


unsigned int Identification::CalcLength()
{
	length = 4+16 +								// InstanceID
	 		 4+16 +								// This GenerationID
			 4+((unsigned int)wcslen(companyName))*2 +		// Company Name
			 4+((unsigned int)wcslen(productName))*2 +		// Product Name
			 (bProductVersion?(4+10):0) +		// Product Version
			 4+((unsigned int)wcslen(versionString))*2 +	// Version String
			 4+16 +								// Product ID
			 4+8 +								// Modification Date
			 (bToolkitVersion?(4+10):0) +		// Toolkit Version
			 (bPlatform?(4+((unsigned int)wcslen(platform))*2):0);	// Platform

	return length;
}


unsigned int Identification::Write(IWriter* pWriter)
{
DWORD nWritten;

	CalcLength();

	nWritten = WriteBuffer(key, 16, pWriter);
	nWritten += WriteBERLength(length, pWriter);
	
	nWritten += WriteNumber16(0x3c0a, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteBuffer(instanceUID, 16, pWriter);

	nWritten += WriteNumber16(0x3c09, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteBuffer(thisGenerationUID, 16, pWriter);

	nWritten += WriteNumber16(0x3c01, pWriter);
	nWritten += WriteNumber16((unsigned int)wcslen(companyName)*2, pWriter);
	nWritten += WriteBuffer(companyName, (unsigned int)wcslen(companyName)*2, pWriter);

	nWritten += WriteNumber16(0x3c02, pWriter);
	nWritten += WriteNumber16((unsigned int)wcslen(productName)*2, pWriter);
	nWritten += WriteBuffer(productName, (unsigned int)wcslen(productName)*2, pWriter);

	if(bProductVersion)
	{
		nWritten += WriteNumber16(0x3c03, pWriter);
		nWritten += WriteNumber16(10, pWriter);
		nWritten += WriteNumber16(productVersion[0], pWriter);
		nWritten += WriteNumber16(productVersion[1], pWriter);
		nWritten += WriteNumber16(productVersion[2], pWriter);
		nWritten += WriteNumber16(productVersion[3], pWriter);
		nWritten += WriteNumber16(productVersion[4], pWriter);
	}

	nWritten += WriteNumber16(0x3c04, pWriter);
	nWritten += WriteNumber16((unsigned int)wcslen(versionString)*2, pWriter);
	nWritten += WriteBuffer(versionString, (unsigned int)wcslen(versionString)*2, pWriter);

	nWritten += WriteNumber16(0x3c05, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteBuffer(productUID, 16, pWriter);

	nWritten += WriteNumber16(0x3c06, pWriter);
	nWritten += WriteNumber16(8, pWriter);
	nWritten += WriteNumber64(modificationDate.i64Data, pWriter);

	if(bToolkitVersion)
	{
		nWritten += WriteNumber16(0x3c07, pWriter);
		nWritten += WriteNumber16(10, pWriter);
		nWritten += WriteBuffer(toolkitVersion, 10, pWriter);		
	}

	if(bPlatform)
	{
		nWritten += WriteNumber16(0x3c08, pWriter);
		nWritten += WriteNumber16((unsigned int)wcslen(platform)*2, pWriter);
		nWritten += WriteBuffer(versionString, (unsigned int)wcslen(platform)*2, pWriter);
	}

	return nWritten;
}


BOOL Identification::Read(HANDLE hMXF, long chunkSize)
{
unsigned int localTag;
unsigned int size;
DWORD nRead;

	while(chunkSize > 0)
	{
		localTag = GetNumber16(hMXF);
		size = GetNumber16(hMXF);
		chunkSize -= 4;
		chunkSize -= size;

		switch(localTag)
		{
			case 0x3c0a:
				ReadFile(hMXF, instanceUID, 16, &nRead, NULL);
			break;
			case 0x3c09:
				ReadFile(hMXF, thisGenerationUID, 16, &nRead, NULL);
			break;
			case 0x3c01:
				companyName = new wchar_t[(size/sizeof(wchar_t))+1];
				ReadFile(hMXF, companyName, size, &nRead, NULL);
				companyName[(size/sizeof(wchar_t))] = 0x00;
				InvertUTF16String(companyName);				
			break;
			case 0x3c02:
 				productName = new wchar_t[(size/sizeof(wchar_t))+1];
				ReadFile(hMXF, productName, size, &nRead, NULL);
				productName[(size/sizeof(wchar_t))] = 0x00;
				InvertUTF16String(productName);
			break;
			case 0x3c03:
				bProductVersion = true;
				ReadFile(hMXF, &productVersion, 10, &nRead, NULL);
				productVersion[0] = (productVersion[0]<<8) + (productVersion[0]>>8);
				productVersion[1] = (productVersion[1]<<8) + (productVersion[1]>>8);
				productVersion[2] = (productVersion[2]<<8) + (productVersion[2]>>8);
				productVersion[3] = (productVersion[3]<<8) + (productVersion[3]>>8);
				productVersion[4] = (productVersion[4]<<8) + (productVersion[4]>>8);
			break;
			case 0x3c04:
				versionString = new wchar_t[(size/sizeof(wchar_t))+1];
				ReadFile(hMXF, versionString, size, &nRead, NULL);
				versionString[(size/sizeof(wchar_t))] = 0x00;
				InvertUTF16String(versionString);
			break;
			case 0x3c05:
				ReadFile(hMXF, productUID, 16, &nRead, NULL);
			break;
			case 0x3c06:
				modificationDate.i64Data = GetNumber64(hMXF);
			break;
			case 0x3c07:
				bToolkitVersion = true;
				ReadFile(hMXF, &toolkitVersion, 10, &nRead, NULL);
			break;
			case 0x3c08:
				bPlatform = true;
				platform = new wchar_t[(size/sizeof(wchar_t))+1];
				ReadFile(hMXF, platform, size, &nRead, NULL);
				platform[(size/sizeof(wchar_t))] = 0x00;
				InvertUTF16String(platform);
			break;

			/*default:
				SetFilePointer(hMXF, size, NULL, FILE_CURRENT);*/
		}
	}

	return TRUE;
}



unsigned int ContentStorage::CalcLength()
{
	length = 4+16 + // InstanceUID
			 (bGenerationUID?(4+16):0) + // GenerationUID
			 4+8+packages.num*packages.size + // Packages
			 (bEssenceContainerData?(4+8+essenceContainerData.num*essenceContainerData.size):0); // Essence Container Data

	return length;
}


unsigned int ContentStorage::Write(IWriter* pWriter)
{
DWORD nWritten;
unsigned int i;

	CalcLength();

	nWritten = WriteBuffer(key, 16, pWriter);
	nWritten += WriteBERLength(length, pWriter);

	nWritten += WriteNumber16(0x3c0a, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteBuffer(instanceUID, 16, pWriter);

	if(bGenerationUID)
	{
		nWritten += WriteNumber16(0x0102, pWriter);
		nWritten += WriteNumber16(16, pWriter);
		nWritten += WriteBuffer(generationUID, 16, pWriter);
	}

	nWritten += WriteNumber16(0x1901, pWriter);
	nWritten += WriteNumber16(8+packages.num*packages.size, pWriter);
	nWritten += WriteNumber32(packages.num, pWriter);
	nWritten += WriteNumber32(packages.size, pWriter);
	for(i = 0; i < packages.num; i++)
		nWritten += WriteBuffer(packages.ppItem[i], packages.size, pWriter);

	if(bEssenceContainerData)
	{
		nWritten += WriteNumber16(0x1902, pWriter);
		nWritten += WriteNumber16(8+essenceContainerData.num*essenceContainerData.size, pWriter);
		nWritten += WriteNumber32(essenceContainerData.num, pWriter);
		nWritten += WriteNumber32(essenceContainerData.size, pWriter);
		for(i = 0; i < essenceContainerData.num; i++)
			nWritten += WriteBuffer(essenceContainerData.ppItem[i], essenceContainerData.size, pWriter);
	}

	return nWritten;
}


BOOL ContentStorage::Read(HANDLE hMXF, long chunkSize)
{
unsigned int localTag;
unsigned int size;
DWORD nRead;

	while(chunkSize > 0)
	{
		localTag = GetNumber16(hMXF);
		size = GetNumber16(hMXF);
		chunkSize -= 4;
		chunkSize -= size;

		switch(localTag)
		{
			case 0x3c0a:
				ReadFile(hMXF, instanceUID, 16, &nRead, NULL);
			break;
			case 0x0102:
				bGenerationUID = true;
				ReadFile(hMXF, generationUID, 16, &nRead, NULL);
			break;
			case 0x1901:
				packages.Read(hMXF);
			break;
			case 0x1902:
				bEssenceContainerData = true;
				essenceContainerData.Read(hMXF);
			break;
			default:
				SetFilePointer(hMXF, -4, NULL, FILE_CURRENT);
			break;
		}
	}
	
	return TRUE;
}


unsigned int EssenceContainerData::CalcLength()
{
	length = 4+16 + // Instance UID
			 4+32 + // Linked Package UID
			 (bGenerationUID?(4+16):0) + // Generation UID
			 (bIndexSID?(4+4):0) + // Index SID
			 4+4; // BodySID

	return length;
}


unsigned int EssenceContainerData::Write(IWriter* pWriter)
{
DWORD nWritten;

	CalcLength();

	nWritten = WriteBuffer(key, 16, pWriter);
	nWritten += WriteBERLength(length, pWriter);

	nWritten += WriteNumber16(0x3c0a, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteBuffer(instanceUID, 16, pWriter);

	nWritten += WriteNumber16(0x2701, pWriter);
	nWritten += WriteNumber16(32, pWriter);
	nWritten += WriteBuffer(linkedPackageUID, 32, pWriter);

	if(bGenerationUID)
	{
		nWritten += WriteNumber16(0x0102, pWriter);
		nWritten += WriteNumber16(16, pWriter);
		nWritten += WriteBuffer(generationUID, 16, pWriter);
	}

	if(bIndexSID)
	{
		nWritten += WriteNumber16(0x3f06, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(indexSID, pWriter);
	}

	nWritten += WriteNumber16(0x3f07, pWriter);
	nWritten += WriteNumber16(4, pWriter);
	nWritten += WriteNumber32(bodySID, pWriter);

	return nWritten;
}


BOOL EssenceContainerData::Read(HANDLE hMXF, DWORD chunkSize)
{
unsigned int localTag;
unsigned int size;
DWORD nRead;

	while(chunkSize > 0)
	{
		localTag = GetNumber16(hMXF);
		size = GetNumber16(hMXF);
		chunkSize -= 4;
		chunkSize -= size;

		switch(localTag)
		{
			case 0x3c0a:
				ReadFile(hMXF, instanceUID, 16, &nRead, NULL);
			break;
			case 0x2701:
				ReadFile(hMXF, linkedPackageUID, 32, &nRead, NULL);
			break;
			case 0x0102:
				bGenerationUID = true;
				ReadFile(hMXF, generationUID, 16, &nRead, NULL);
			break;
			case 0x3f06:
				bIndexSID = true;
				indexSID = GetNumber32(hMXF);
			break;
			case 0x3f07:
				bodySID = GetNumber32(hMXF);
			break;

			default:
				SetFilePointer(hMXF, size, NULL, FILE_CURRENT);
		}
	}

	return TRUE;
}



unsigned int GenericPackage::CalcLength()
{
	length = 4+16 + // instance UID
			 4+32 + // package UID
			 (bGenerationUID ? (4+16) : 0) + // Generation UID
			 (bName?(4 + (unsigned int)wcslen(name)*2):0) + // Name
			 4+8 + // PackageCreationDate
			 4+8 + // PackageModifiedDate
			 4+8+tracks.num*tracks.size; // Tracks

	return length;
}


unsigned int GenericPackage::Write(IWriter* pWriter, BOOL bWriteKL)
{
DWORD nWritten = 0;
unsigned int i;

	CalcLength();

	if(bWriteKL)
	{
		nWritten += WriteBuffer(key, 16, pWriter);
		nWritten += WriteBERLength(length, pWriter);
	}

	nWritten += WriteNumber16(0x3c0a, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteBuffer(instanceUID, 16, pWriter);

	nWritten += WriteNumber16(0x4401, pWriter);
	nWritten += WriteNumber16(32, pWriter);
	nWritten += WriteBuffer(packageUID, 32, pWriter);

	if(bGenerationUID)
	{
		nWritten += WriteNumber16(0x0102, pWriter);
		nWritten += WriteNumber16(16, pWriter);
		nWritten += WriteBuffer(generationUID, 16, pWriter);
	}

	if(bName)
	{
		nWritten += WriteNumber16(0x4402, pWriter);
		nWritten += WriteNumber16((unsigned int)wcslen(name)*2, pWriter);
		nWritten += WriteBuffer(name, (unsigned int)wcslen(name)*2, pWriter);
	}

	nWritten += WriteNumber16(0x4405, pWriter);
	nWritten += WriteNumber16(8, pWriter);
	nWritten += WriteNumber64(packageCreationDate.i64Data, pWriter);

	nWritten += WriteNumber16(0x4404, pWriter);
	nWritten += WriteNumber16(8, pWriter);
	nWritten += WriteNumber64(packageModifiedDate.i64Data, pWriter);

	nWritten += WriteNumber16(0x4403, pWriter);
	nWritten += WriteNumber16(8+tracks.num*tracks.size, pWriter);
	nWritten += WriteNumber32(tracks.num, pWriter);
	nWritten += WriteNumber32(tracks.size, pWriter);
	for(i = 0; i < tracks.num; i++)
		nWritten += WriteBuffer(tracks.ppItem[i], tracks.size, pWriter);

	nWritten += WriteNumber16(0x4701, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteBuffer(descriptor, 16, pWriter);

	return nWritten;
}

BOOL GenericPackage::Read(HANDLE hMXF, long chunkSize)
{
unsigned int localTag;
unsigned int size;
DWORD nRead;

	while(chunkSize > 0)
	{
		localTag = GetNumber16(hMXF);
		size = GetNumber16(hMXF);
		chunkSize -= 4;
		chunkSize -= size;

		switch(localTag)
		{
			case 0x3c0a:
				ReadFile(hMXF, instanceUID, 16, &nRead, NULL);
			break;
			case 0x4401:
				ReadFile(hMXF, packageUID, 32, &nRead, NULL);
			break;
			case 0x0102:
				bGenerationUID = true;
				ReadFile(hMXF, generationUID, 16, &nRead, NULL);
			break;
			case 0x4402:
				bName= true;
				name = new wchar_t[(size/sizeof(wchar_t))+1];
				ReadFile(hMXF, name, size, &nRead, NULL);
				name[(size/sizeof(wchar_t))] = 0x00;
				InvertUTF16String(name);
			break;
			case 0x4405:
				packageCreationDate.i64Data = GetNumber64(hMXF);
			break;
			case 0x4404:
				packageModifiedDate.i64Data = GetNumber64(hMXF);
			break;
			case 0x4403:
				tracks.Read(hMXF);
			break;
			case 0x4701:
				ReadFile(hMXF, descriptor, size, &nRead, NULL);
			break;
			default:
				SetFilePointer(hMXF, size, NULL, FILE_CURRENT);
		}
	}

	return TRUE;
}



unsigned int Track::CalcLength()
{
	length = 4+16 + // InstanceUID
			(bGenerationUID?(4+16):0) + // GenerationUID
			(bTrackID?(4+4):0) + // TrackID
			4+4 + // TrackNumber
			(bTrackName?(4+(unsigned int)wcslen(trackName)*2):0) + // Track Name
			4+8 + // Edit Rate
			4+8 + // Origin
			4+16; // Sequence

	return length;
}


unsigned int Track::Write(IWriter* pWriter)
{
DWORD nWritten;
	CalcLength();

	nWritten = WriteBuffer(key, 16, pWriter);
	nWritten += WriteBERLength(length, pWriter);

	nWritten += WriteNumber16(0x3c0a, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteBuffer(instanceUID, 16, pWriter);

	if(bGenerationUID)
	{
		nWritten += WriteNumber16(0x0102, pWriter);
		nWritten += WriteNumber16(16, pWriter);
		nWritten += WriteBuffer(generationUID, 16, pWriter);
	}

	if(bTrackID)
	{
		nWritten += WriteNumber16(0x4801, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(trackID, pWriter);
	}

	nWritten += WriteNumber16(0x4804, pWriter);
	nWritten += WriteNumber16(4, pWriter);
	nWritten += WriteNumber32(trackNumber, pWriter);

	if(bTrackName)
	{
		nWritten += WriteNumber16(0x4802, pWriter);
		nWritten += WriteNumber16((unsigned int)wcslen(trackName)*2, pWriter);
		nWritten += WriteBuffer(trackName, (unsigned int)wcslen(trackName)*2, pWriter);
	}

	nWritten += WriteNumber16(0x4b01, pWriter);
	nWritten += WriteNumber16(8, pWriter);
	nWritten += WriteNumber32(editRate.num, pWriter);
	nWritten += WriteNumber32(editRate.den, pWriter);

	nWritten += WriteNumber16(0x4b02, pWriter);
	nWritten += WriteNumber16(8, pWriter);
	nWritten += WriteNumber64(origin, pWriter);

	nWritten += WriteNumber16(0x4803, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteBuffer(sequence, 16, pWriter);

	return nWritten;
}

BOOL Track::Read(HANDLE hMXF, long chunkSize)
{
unsigned int localTag;
unsigned int size;
DWORD nRead;

	while(chunkSize > 0)
	{
		localTag = GetNumber16(hMXF);
		size = GetNumber16(hMXF);
		chunkSize -= 4;
		chunkSize -= size;

		switch(localTag)
		{
			case 0x3c0a:
				ReadFile(hMXF, instanceUID, 16, &nRead, NULL);
			break;
			case 0x0102:
				bGenerationUID = true;
				ReadFile(hMXF, generationUID, 16, &nRead, NULL);
			break;
			case 0x4801:
				bTrackID = true;
				trackID = GetNumber32(hMXF);
			break;
			case 0x4804:
				trackNumber = GetNumber32(hMXF);
			break;
			case 0x4802:
			{
				bTrackName = true;
				trackName = new wchar_t[size/2];
				ReadFile(hMXF, trackName, size, &nRead, NULL);
				InvertUTF16String(trackName);
			}
			break;
			case 0x4b01:
				editRate.num = GetNumber32(hMXF);
				editRate.den = GetNumber32(hMXF);
			break;
			case 0x4b02:
				origin = GetNumber64(hMXF);
			break;
			case 0x4803:
				ReadFile(hMXF, sequence, 16, &nRead, NULL);
			break;
			default:
				SetFilePointer(hMXF, size, NULL, FILE_CURRENT);
			break;
		}
	}

	return TRUE;
}


unsigned int StaticTrack::CalcLength()
{
	length = 4+16 + // InstanceUID
			(bGenerationUID?(4+16):0) + // GenerationUID
			(bTrackID?(4+4):0) + // TrackID
			4+4 + // TrackNumber
			(bTrackName?(4+(unsigned int)wcslen(trackName)*2):0) + // Track Name
			4+16; // Sequence

	return length;
}

unsigned int StaticTrack::Write(IWriter* pWriter)
{
DWORD nWritten;
	CalcLength();

	nWritten = WriteBuffer(key, 16, pWriter);
	nWritten += WriteBERLength(length, pWriter);

	nWritten += WriteNumber16(0x3c0a, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteBuffer(instanceUID, 16, pWriter);

	if(bGenerationUID)
	{
		nWritten += WriteNumber16(0x0102, pWriter);
		nWritten += WriteNumber16(16, pWriter);
		nWritten += WriteBuffer(generationUID, 16, pWriter);
	}

	if(bTrackID)
	{
		nWritten += WriteNumber16(0x4801, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(trackID, pWriter);
	}

	nWritten += WriteNumber16(0x4804, pWriter);
	nWritten += WriteNumber16(4, pWriter);
	nWritten += WriteNumber32(trackNumber, pWriter);

	if(bTrackName)
	{
		nWritten += WriteNumber16(0x4802, pWriter);
		nWritten += WriteNumber16((unsigned int)wcslen(trackName)*2, pWriter);
		nWritten += WriteBuffer(trackName, (unsigned int)wcslen(trackName)*2, pWriter);
	}


	nWritten += WriteNumber16(0x4803, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteBuffer(sequence, 16, pWriter);

	return nWritten;
}

BOOL StaticTrack::Read(HANDLE hMXF, DWORD chunkSize)
{
unsigned int localTag;
unsigned int size;
DWORD nRead;


	while(chunkSize > 0)
	{
		localTag = GetNumber16(hMXF);
		size = GetNumber16(hMXF);
		chunkSize -= 4;
		chunkSize -= size;

		switch(localTag)
		{
			case 0x3c0a:
				ReadFile(hMXF, instanceUID, 16, &nRead, NULL);
			break;
			case 0x0102:
				bGenerationUID = true;
				ReadFile(hMXF, generationUID, 16, &nRead, NULL);
			break;
			case 0x4801:
				bTrackID = true;
				trackID = GetNumber32(hMXF);
			break;
			case 0x4804:
				trackNumber = GetNumber32(hMXF);
			break;
			case 0x4802:
				bTrackName = true;
				trackName = new wchar_t[size/2];
				ReadFile(hMXF, trackName, size, &nRead, NULL);
				InvertUTF16String(trackName);
			break;
			case 0x4803:
				ReadFile(hMXF, sequence, 16, &nRead, NULL);
			break;
			default:
				SetFilePointer(hMXF, size, NULL, FILE_CURRENT);
		}
	}

	return TRUE;
}


unsigned int Sequence::CalcLength()
{
	length = 4+16 + // InstanceID
			 (bGenerationUID?(4+16):0) + // GenerationUID
			 4+16 + // Data Definition
			 4+8 + // duration
			 4+8+structuralComponents.num*structuralComponents.size;

	return length;
}


unsigned int Sequence::Write(IWriter* pWriter)
{
unsigned int i;
DWORD nWritten;

	CalcLength();

	nWritten = WriteBuffer(key, 16, pWriter);
	nWritten += WriteBERLength(length, pWriter);

	nWritten += WriteNumber16(0x3c0a, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteBuffer(instanceUID, 16, pWriter);

	if(bGenerationUID)
	{
		nWritten += WriteNumber16(0x0102, pWriter);
		nWritten += WriteNumber16(16, pWriter);
		nWritten += WriteBuffer(generationUID, 16, pWriter);
	}

	nWritten += WriteNumber16(0x0201, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteBuffer(dataDefinition, 16, pWriter);

	nWritten += WriteNumber16(0x0202, pWriter);
	nWritten += WriteNumber16(8, pWriter);
	nWritten += WriteNumber64(duration, pWriter);

	nWritten += WriteNumber16(0x1001, pWriter);
	nWritten += WriteNumber16(8+structuralComponents.num*structuralComponents.size, pWriter);
	nWritten += WriteNumber32(structuralComponents.num, pWriter);
	nWritten += WriteNumber32(structuralComponents.size, pWriter);
	for(i = 0; i < structuralComponents.num; i++)
		nWritten += WriteBuffer(structuralComponents.ppItem[i], structuralComponents.size, pWriter);

	return nWritten;
}


BOOL Sequence::Read(HANDLE hMXF, long chunkSize)
{
unsigned int localTag;
unsigned int size;
DWORD nRead;

	while(chunkSize > 0)
	{
		localTag = GetNumber16(hMXF);
		size = GetNumber16(hMXF);
		chunkSize -= 4;
		chunkSize -= size;

		switch(localTag)
		{
			case 0x3c0a:
				ReadFile(hMXF, instanceUID, 16, &nRead, NULL);
			break;
			case 0x0102:
				bGenerationUID = true;
				ReadFile(hMXF, generationUID, 16, &nRead, NULL);
			break;
			case 0x0201:
				ReadFile(hMXF, dataDefinition, 16, &nRead, NULL);
			break;
			case 0x0202:
				duration = GetNumber64(hMXF);
			break;
			case 0x1001:
				structuralComponents.num = GetNumber32(hMXF);
				structuralComponents.size = GetNumber32(hMXF);
				structuralComponents.ppItem = new unsigned char*[structuralComponents.num];
				for(unsigned int i=0; i < structuralComponents.num; i++)
				{
					structuralComponents.ppItem[i] = new unsigned char[structuralComponents.size];
					ReadFile(hMXF, structuralComponents.ppItem[i], structuralComponents.size, &nRead, NULL);
				}			
			break;

			default:
				SetFilePointer(hMXF, size, NULL, FILE_CURRENT);
		}
	}
	
	return TRUE;
}


unsigned int SourceClip::CalcLength()
{
	length = 4+16 + // InstanceID
			 (bGenerationUID?(4+16):0) + // GenerationID
			 4+16 + // Data Definition
			 4+8 + // Start Position
			 4+8 + // Duration
			 4+32 + // Source Package ID
			 4+4; // Source Track ID

	return length;
}


unsigned int SourceClip::Write(IWriter* pWriter)
{
DWORD nWritten;

	CalcLength();

	nWritten = WriteBuffer(key, 16, pWriter);
	nWritten += WriteBERLength(length, pWriter);

	nWritten += WriteNumber16(0x3c0a, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteBuffer(instanceUID, 16, pWriter);

	if(bGenerationUID)
	{
		nWritten += WriteNumber16(0x0102, pWriter);
		nWritten += WriteNumber16(16, pWriter);
		nWritten += WriteBuffer(generationUID, 16, pWriter);
	}

	nWritten += WriteNumber16(0x0201, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteBuffer(dataDefinition, 16, pWriter);

	nWritten += WriteNumber16(0x1201, pWriter);
	nWritten += WriteNumber16(8, pWriter);
	nWritten += WriteNumber64(startPosition, pWriter);

	nWritten += WriteNumber16(0x0202, pWriter);
	nWritten += WriteNumber16(8, pWriter);
	nWritten += WriteNumber64(duration, pWriter);

	nWritten += WriteNumber16(0x1101, pWriter);
	nWritten += WriteNumber16(32, pWriter);
	nWritten += WriteBuffer(sourcePackageID, 32, pWriter);

	nWritten += WriteNumber16(0x1102, pWriter);
	nWritten += WriteNumber16(4, pWriter);
	nWritten += WriteNumber32(sourceTrackID, pWriter);

	return nWritten;
}


BOOL SourceClip::Read(HANDLE hMXF, DWORD chunkSize)
{
unsigned int localTag;
unsigned int size;
DWORD nRead;

	while(chunkSize > 0)
	{
		localTag = GetNumber16(hMXF);
		size = GetNumber16(hMXF);
		chunkSize -= 4;
		chunkSize -= size;

		switch(localTag)
		{
			case 0x3c0a:
				ReadFile(hMXF, instanceUID, 16, &nRead, NULL);
			break;
			case 0x0102:
				bGenerationUID = true;
				ReadFile(hMXF, generationUID, 16, &nRead, NULL);
			break;
			case 0x0201:
				ReadFile(hMXF, dataDefinition, 16, &nRead, NULL);
			break;
			case 0x1201:
				startPosition = GetNumber64(hMXF);
			break;
			case 0x0202:
				duration = GetNumber64(hMXF);
			break;
			case 0x1101:
				bSourcePackageID = true;
				ReadFile(hMXF, sourcePackageID, 32, &nRead, NULL);
			break;
			case 0x1102:
				sourceTrackID = GetNumber32(hMXF);
			break;

			default:
				SetFilePointer(hMXF, size, NULL, FILE_CURRENT);
		}
	}


	return TRUE;
}


unsigned int TimecodeComponent::CalcLength()
{
	length = 4+16 + // InstanceUID
			 (bGenerationUID?(4+16):0) + // GenerationUID
			 4+16 + // Data Definition
			 4+8 + // Duration
			 4+2 + // Rounded Timecode Base
			 4+8 + // Start Timecode
			 4+1; // Dropframe

	return length;
}


unsigned int TimecodeComponent::Write(IWriter* pWriter)
{
DWORD nWritten;

	CalcLength();

	nWritten = WriteBuffer(key, 16, pWriter);
	nWritten += WriteBERLength(length, pWriter);

	nWritten += WriteNumber16(0x3c0a, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteBuffer(instanceUID, 16, pWriter);

	if(bGenerationUID)
	{
		nWritten += WriteNumber16(0x0102, pWriter);
		nWritten += WriteNumber16(16, pWriter);
		nWritten += WriteBuffer(generationUID, 16, pWriter);
	}

	nWritten += WriteNumber16(0x0201, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteBuffer(dataDefinition, 16, pWriter);

	nWritten += WriteNumber16(0x0202, pWriter);
	nWritten += WriteNumber16(8, pWriter);
	nWritten += WriteNumber64(duration, pWriter);

	nWritten += WriteNumber16(0x1502, pWriter);
	nWritten += WriteNumber16(2, pWriter);
	nWritten += WriteNumber16(roundedTimecodeBase, pWriter);

	nWritten += WriteNumber16(0x1501, pWriter);
	nWritten += WriteNumber16(8, pWriter);
	nWritten += WriteNumber64(startTimecode, pWriter);

	nWritten += WriteNumber16(0x1503, pWriter);
	nWritten += WriteNumber16(1, pWriter);
	nWritten += WriteNumber8(dropFrame, pWriter);

	return nWritten;
}


BOOL TimecodeComponent::Read(HANDLE hMXF, long chunkSize)
{
unsigned int localTag;
unsigned int size;
DWORD nRead;

	while(chunkSize > 0)
	{
		localTag = GetNumber16(hMXF);
		size = GetNumber16(hMXF);
		chunkSize -= 4;
		chunkSize -= size;

		switch(localTag)
		{
			case 0x3c0a:
				ReadFile(hMXF, instanceUID, 16, &nRead, NULL);
			break;
			case 0x0102:
				bGenerationUID = true;
				ReadFile(hMXF, generationUID, 16, &nRead, NULL);
			break;
			case 0x0201:
				ReadFile(hMXF, dataDefinition, 16, &nRead, NULL);
			break;
			case 0x0202:
				duration = GetNumber64(hMXF);
			break;
			case 0x1502:
				roundedTimecodeBase = GetNumber16(hMXF);
			break;
			case 0x1501:
				startTimecode = GetNumber64(hMXF);
			break;
			case 0x1503:
				dropFrame = GetNumber8(hMXF);
			break;

			default:
				SetFilePointer(hMXF, size, NULL, FILE_CURRENT);
		}
	}

	return TRUE;
}


unsigned int SourcePackage::CalcLength()
{
	length = package.CalcLength() + 
			 4+16; // DESCRIPTOR

	return length;
}

unsigned int SourcePackage::Write(IWriter* pWriter)
{
DWORD nWritten = 0;

	CalcLength();

	nWritten = WriteBuffer(key, 16, pWriter);
	nWritten += WriteBERLength(length, pWriter);

	nWritten += package.Write(pWriter, FALSE);
	return nWritten;
}


BOOL SourcePackage::Read(HANDLE hMXF, long chunkSize)
{
	//package.Read(hMXF, chunkSize-20);
	package.Read(hMXF, chunkSize);
	return TRUE;
}

unsigned int FileDescriptor::CalcLength()
{
	length = 4+16 + // Instance ID
			 (bGenerationUID?(4+16):0) + // Generation UID
			 (bLinkedTrackID?(4+4):0) + // Linked Track ID
			 4+8 + // Sample Rate
			 (bContainerDuration?(4+8):0) + // Container Duration
			 4+16 + // Essence Container
			 (bCodec?(4+16) : 0) + // Codec
			 (bLocators?(4+8+locators.num*locators.size):0); // Locators

	return length;
}


unsigned int FileDescriptor::Write(IWriter* pWriter, BOOL bWriteKL)
{
DWORD nWritten = 0;
unsigned int i;

	CalcLength();

	if(bWriteKL)
	{
		nWritten += WriteBuffer(key, 16, pWriter);
		nWritten += WriteBERLength(length, pWriter);
	}

	nWritten += WriteNumber16(0x3c0a, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteBuffer(instanceUID, 16, pWriter);

	if(bGenerationUID)
	{
		nWritten += WriteNumber16(0x0102, pWriter);
		nWritten += WriteNumber16(16, pWriter);
		nWritten += WriteBuffer(generationUID, 16, pWriter);
	}

	if(bLinkedTrackID)
	{
		nWritten += WriteNumber16(0x3006, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(linkedTrackID, pWriter);
	}

	nWritten += WriteNumber16(0x3001, pWriter);
	nWritten += WriteNumber16(8, pWriter);
	nWritten += WriteNumber32(sampleRate.num, pWriter);
	nWritten += WriteNumber32(sampleRate.den, pWriter);

	if(bContainerDuration)
	{
		nWritten += WriteNumber16(0x3002, pWriter);
		nWritten += WriteNumber16(8, pWriter);
		nWritten += WriteNumber64(containerDuration, pWriter);
	}

	nWritten += WriteNumber16(0x3004, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteBuffer(essenceContainer, 16, pWriter);

	if(bCodec)
	{
		nWritten += WriteNumber16(0x3005, pWriter);
		nWritten += WriteNumber16(16, pWriter);
		nWritten += WriteBuffer(codec, 16, pWriter);
	}

	if(bLocators)
	{
		nWritten += WriteNumber16(0x2f01, pWriter);
		nWritten += WriteNumber16(8+locators.num*locators.size, pWriter);
		nWritten += WriteNumber32(locators.num, pWriter);
		nWritten += WriteNumber32(locators.size, pWriter);
		for(i = 0; i < locators.num; i++)
			nWritten += WriteBuffer(locators.ppItem[i], locators.size, pWriter);
	}

	return nWritten;
}


BOOL FileDescriptor::Read(HANDLE hMXF, long chunkSize)
{
unsigned int localTag;
unsigned int size;
DWORD nRead;	

	while(chunkSize > 0)
	{
		localTag = GetNumber16(hMXF);
		size = GetNumber16(hMXF);
		chunkSize -= 4;
		chunkSize -= size;

		switch(localTag)
		{
			case 0x3c0a:
				ReadFile(hMXF, instanceUID, 16, &nRead, NULL);
			break;
			case 0x0102:
				bGenerationUID = true;
				ReadFile(hMXF, generationUID, 16, &nRead, NULL);
			break;
			case 0x3006:
				bLinkedTrackID = true;
				linkedTrackID = GetNumber32(hMXF);
			break;
			case 0x3001:
				sampleRate.num = GetNumber32(hMXF);
				sampleRate.den = GetNumber32(hMXF);
			break;
			case 0x3002:
				bContainerDuration = true;
				containerDuration = GetNumber64(hMXF);
			break;
			case 0x3004:
				ReadFile(hMXF, essenceContainer, 16, &nRead, NULL);
			break;
			case 0x3005:
				bCodec = true;
				ReadFile(hMXF, codec, 16, &nRead, NULL);
			break;
			case 0x2f01:
				bLocators = true;
				locators.Read(hMXF);
			break;
			default:
				SetFilePointer(hMXF, size, NULL, FILE_CURRENT);
			break;
		}
	}

	return TRUE;
}


unsigned int MultipleDescriptor::CalcLength()
{
	length = fileDescriptor.CalcLength() + // File Descriptor
			 4+8+subDescriptorUIDs.num*subDescriptorUIDs.size; // Sub Descriptor UIDs

	return length;
}


unsigned int MultipleDescriptor::Write(IWriter* pWriter)
{
unsigned int i;
DWORD nWritten;

	CalcLength();

	nWritten = WriteBuffer(key, 16, pWriter);
	nWritten += WriteBERLength(length, pWriter);

	nWritten += fileDescriptor.Write(pWriter, FALSE);

	nWritten += WriteNumber16(0x3f01, pWriter);
	nWritten += WriteNumber16(8+subDescriptorUIDs.num*subDescriptorUIDs.size, pWriter);
	nWritten += WriteNumber32(subDescriptorUIDs.num, pWriter);
	nWritten += WriteNumber32(subDescriptorUIDs.size, pWriter);
	for(i = 0; i < subDescriptorUIDs.num; i++)
		nWritten += WriteBuffer(subDescriptorUIDs.ppItem[i], subDescriptorUIDs.size, pWriter);

	return nWritten;
}


BOOL MultipleDescriptor::Read(HANDLE hMXF, DWORD chunkSize)
{
unsigned int localTag;
unsigned int size;
DWORD nRead;


	while(chunkSize > 0)
	{
		localTag = GetNumber16(hMXF);
		size = GetNumber16(hMXF);
		chunkSize -= 4;
		chunkSize -= size;

		switch(localTag)
		{
			// FILE DESCRIPTOR
			case 0x3c0a:
				ReadFile(hMXF, fileDescriptor.instanceUID, 16, &nRead, NULL);
			break;
			case 0x0102:
				fileDescriptor.bGenerationUID = true;
				ReadFile(hMXF, fileDescriptor.generationUID, 16, &nRead, NULL);
			break;
			case 0x3006:
				fileDescriptor.bLinkedTrackID = true;
				fileDescriptor.linkedTrackID = GetNumber32(hMXF);
			break;
			case 0x3001:
				fileDescriptor.sampleRate.num = GetNumber32(hMXF);
				fileDescriptor.sampleRate.den = GetNumber32(hMXF);
			break;
			case 0x3002:
				fileDescriptor.bContainerDuration = true;
				fileDescriptor.containerDuration = GetNumber64(hMXF);
			break;
			case 0x3004:
				ReadFile(hMXF, fileDescriptor.essenceContainer, 16, &nRead, NULL);
			break;
			case 0x3005:
				fileDescriptor.bCodec = true;
				ReadFile(hMXF, fileDescriptor.codec, 16, &nRead, NULL);
			break;
			case 0x2f01:
				fileDescriptor.bLocators = true;
				fileDescriptor.locators.Read(hMXF);
			break;

			// MULTIPLE DESCRIPTOR
			case 0x3f01:
				subDescriptorUIDs.Read(hMXF);
			break;

			default:
				SetFilePointer(hMXF, size, NULL, FILE_CURRENT);
		}
	}

	return TRUE;
}


unsigned int GenericPictureEssenceDescriptor::CalcLength()
{
	length = fileDescriptor.CalcLength() + // File Descriptor
			 (bSignalStandard?(4+1):0) + // Signal Standard
			 4+1 + // Frame Layout
			 4+4 + // Stored Width
			 4+4 + // Stored Height
			 (bStoredF2Offset?(4+4):0) + // Stored F2 Offset
			 (bSampledWidth?(4+4):0) + // Sampled Width
			 (bSampledHeight?(4+4):0) + // Sampled Height
			 (bSampledXOffset?(4+4):0) + // Sampled X Offset
			 (bSampledYOffset?(4+4):0) + // Sampled Y Offset
			 (bDisplayHeight?(4+4):0) + // DisplayHeight
			 (bDisplayWidth?(4+4):0) + // DisplayWidth
			 (bDisplayXOffset?(4+4):0) + // Display X Offset
			 (bDisplayYOffset?(4+4):0) + // Display Y Offset
			 (bDisplayF2Offset?(4+4):0) + // Display F2 Offset
			 4+8 + // Aspect Ratio
			 (bActiveFormatDescriptor?(4+1):0) + // Active Format Descriptor
			 4+8+8 + // Video Line Map
			 (bAlphaTransparency?(4+1):0) + // Alpha Transparency
			 (bCaptureGamma?(4+16):0) + // Capture Gamma
			 (bImageAlignmentOffset?(4+4):0) + // Image Alignment Offset
			 (bImageStartOffset?(4+4):0) + // Image Start Offset
			 (bImageEndOffset?(4+4):0) + // Image End Offset
			 (bFieldDominance?(4+1):0) + // Field Dominance
			 (bPictureEssenceCoding?(4+16):0); // Picture Essence Coding


	return length;
}


unsigned int GenericPictureEssenceDescriptor::Write(IWriter* pWriter, BOOL bWriteKL)
{
DWORD nWritten = 0;

	CalcLength();

	if(bWriteKL)
	{
		nWritten += WriteBuffer(key, 16, pWriter);
		nWritten += WriteBERLength(length, pWriter);
	}

	nWritten += fileDescriptor.Write(pWriter, FALSE);

	if(bSignalStandard)
	{
		nWritten += WriteNumber16(0x3215, pWriter);
		nWritten += WriteNumber16(1, pWriter);
		nWritten += WriteNumber8(signalStandard, pWriter);
	}

	nWritten += WriteNumber16(0x320c, pWriter);
	nWritten += WriteNumber16(1, pWriter);
	nWritten += WriteNumber8(frameLayout, pWriter);

	nWritten += WriteNumber16(0x3203, pWriter);
	nWritten += WriteNumber16(4, pWriter);
	nWritten += WriteNumber32(storedWidth, pWriter);

	nWritten += WriteNumber16(0x3202, pWriter);
	nWritten += WriteNumber16(4, pWriter);
	nWritten += WriteNumber32(storedHeight, pWriter);

	if(bStoredF2Offset)
	{
		nWritten += WriteNumber16(0x3216, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(storedF2Offset, pWriter);
	}

	if(bSampledWidth)
	{
		nWritten += WriteNumber16(0x3205, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(sampledWidth, pWriter);
	}

	if(bSampledHeight)
	{
		nWritten += WriteNumber16(0x3204, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(sampledHeight, pWriter);
	}

	if(bSampledXOffset)
	{
		nWritten += WriteNumber16(0x3206, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(sampledXOffset, pWriter);
	}

	if(bSampledYOffset)
	{
		nWritten += WriteNumber16(0x3207, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(sampledYOffset, pWriter);
	}

	if(bDisplayHeight)
	{
		nWritten += WriteNumber16(0x3208, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(displayHeight, pWriter);
	}

	if(bDisplayWidth)
	{
		nWritten += WriteNumber16(0x3209, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(displayWidth, pWriter);
	}

	if(bDisplayXOffset)
	{
		nWritten += WriteNumber16(0x320a, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(displayXOffset, pWriter);
	}

	if(bDisplayYOffset)
	{
		nWritten += WriteNumber16(0x320b, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(displayYOffset, pWriter);
	}

	if(bDisplayF2Offset)
	{
		nWritten += WriteNumber16(0x3217, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(displayF2Offset, pWriter);
	}

	nWritten += WriteNumber16(0x320e, pWriter);
	nWritten += WriteNumber16(8, pWriter);
	nWritten += WriteNumber32(aspectRatio.num, pWriter);
	nWritten += WriteNumber32(aspectRatio.den, pWriter);

	if(bActiveFormatDescriptor)
	{
		nWritten += WriteNumber16(0x3218, pWriter);
		nWritten += WriteNumber16(1, pWriter);
		nWritten += WriteNumber8(activeFormatDescriptor, pWriter);
	}

	nWritten += WriteNumber16(0x320d, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteNumber32(2, pWriter);
	nWritten += WriteNumber32(4, pWriter);
	nWritten += WriteNumber32(videoLineMap[0], pWriter);
	nWritten += WriteNumber32(videoLineMap[1], pWriter);

	if(bAlphaTransparency)
	{
		nWritten += WriteNumber16(0x320f, pWriter);
		nWritten += WriteNumber16(1, pWriter);
		nWritten += WriteNumber8(alphaTransparency, pWriter);
	}

	if(bCaptureGamma)
	{
		nWritten += WriteNumber16(0x3210, pWriter);
		nWritten += WriteNumber16(16, pWriter);
		nWritten += WriteBuffer(captureGamma, 16, pWriter);
	}

	if(bImageAlignmentOffset)
	{
		nWritten += WriteNumber16(0x3211, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(imageAlignmentOffset, pWriter);
	}

	if(bImageStartOffset)
	{
		nWritten += WriteNumber16(0x3213, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(imageStartOffset, pWriter);
	}

	if(bImageEndOffset)
	{
		nWritten += WriteNumber16(0x3214, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(imageEndOffset, pWriter);
	}

	if(bFieldDominance)
	{
		nWritten += WriteNumber16(0x3212, pWriter);
		nWritten += WriteNumber16(1, pWriter);
		nWritten += WriteNumber8(fieldDominance, pWriter);
	}

	if(bPictureEssenceCoding)
	{
		nWritten += WriteNumber16(0x3201, pWriter);
		nWritten += WriteNumber16(16, pWriter);
		nWritten += WriteBuffer(pictureEssenceCoding, 16, pWriter);
	}

	return nWritten;
}


BOOL GenericPictureEssenceDescriptor::Read(HANDLE hMXF, DWORD chunkSize)
{
unsigned int localTag;
unsigned int size;
DWORD nRead;

//	fileDescriptor.Read(hMXF);

	while(chunkSize > 0)
	{
		localTag = GetNumber16(hMXF);
		size = GetNumber16(hMXF);
		chunkSize -= 4;
		chunkSize -= size;

		switch(localTag)
		{
			// FILE DESCRIPTOR
			case 0x3c0a:
				ReadFile(hMXF, fileDescriptor.instanceUID, 16, &nRead, NULL);
			break;
			case 0x0102:
				fileDescriptor.bGenerationUID = true;
				ReadFile(hMXF, fileDescriptor.generationUID, 16, &nRead, NULL);
			break;
			case 0x3006:
				fileDescriptor.bLinkedTrackID = true;
				fileDescriptor.linkedTrackID = GetNumber32(hMXF);
			break;
			case 0x3001:
				fileDescriptor.sampleRate.num = GetNumber32(hMXF);
				fileDescriptor.sampleRate.den = GetNumber32(hMXF);
			break;
			case 0x3002:
				fileDescriptor.bContainerDuration = true;
				fileDescriptor.containerDuration = GetNumber64(hMXF);
			break;
			case 0x3004:
				ReadFile(hMXF, fileDescriptor.essenceContainer, 16, &nRead, NULL);
			break;
			case 0x3005:
				fileDescriptor.bCodec = true;
				ReadFile(hMXF, fileDescriptor.codec, 16, &nRead, NULL);
			break;
			case 0x2f01:
				fileDescriptor.bLocators = true;
				fileDescriptor.locators.Read(hMXF);
			break;

			// GENERIC PICTURE ESSENCE DESCRIPTOR
			case 0x3215:
				bSignalStandard = true;
				signalStandard = GetNumber8(hMXF);
			break;
			case 0x320c:
				frameLayout = GetNumber8(hMXF);
			break;
			case 0x3203:
				storedWidth = GetNumber32(hMXF);
			break;
			case 0x3202:
				storedHeight = GetNumber32(hMXF);
			break;
			case 0x3216:
				bStoredF2Offset = true;
				storedF2Offset = (Int32)GetNumber32(hMXF);
			break;
			case 0x3205:
				bSampledWidth = true;
				sampledWidth = GetNumber32(hMXF);
			break;
			case 0x3204:
				bSampledHeight = true;
				sampledHeight = GetNumber32(hMXF);
			break;
			case 0x3206:
				bSampledXOffset = true;
				sampledXOffset = (Int32)GetNumber32(hMXF);
			break;
			case 0x3207:
				bSampledYOffset = true;
				sampledYOffset = (Int32)GetNumber32(hMXF);
			break;
			case 0x3208:
				bDisplayHeight = true;
				displayHeight = GetNumber32(hMXF);
			break;
			case 0x3209:
				bDisplayWidth = true;
				displayWidth = GetNumber32(hMXF);
			break;
			case 0x320a:
				bDisplayXOffset = true;
				displayXOffset = (Int32)GetNumber32(hMXF);
			break;
			case 0x320b:
				bDisplayYOffset = true;
				displayYOffset = (Int32)GetNumber32(hMXF);
			break;
			case 0x3217:
				bDisplayF2Offset = true;
				displayF2Offset = (Int32)GetNumber32(hMXF);
			break;
			case 0x320e:
				aspectRatio.num = GetNumber32(hMXF);
				aspectRatio.den = GetNumber32(hMXF);
			break;
			case 0x3218:
				bActiveFormatDescriptor = true;
				activeFormatDescriptor = GetNumber8(hMXF);
			break;
			case 0x320d:
				GetNumber32(hMXF);
				GetNumber32(hMXF);
				videoLineMap[0] = GetNumber32(hMXF);
				videoLineMap[1] = GetNumber32(hMXF);
			break;
			case 0x320f:
				bAlphaTransparency = true;
				alphaTransparency = GetNumber8(hMXF);
			break;
			case 0x3210:
				bCaptureGamma = true;
				ReadFile(hMXF, captureGamma, 16, &nRead, NULL);
			break;
			case 0x3211:
				bImageAlignmentOffset = true;
				imageAlignmentOffset = GetNumber32(hMXF);
			break;
			case 0x3213:
				bImageStartOffset = true;
				imageStartOffset = GetNumber32(hMXF);
			break;
			case 0x3214:
				bImageEndOffset = true;
				imageEndOffset = GetNumber32(hMXF);
			break;
			case 0x3212:
				bFieldDominance = true;
				fieldDominance = GetNumber8(hMXF);
			break;
			case 0x3201:
				bPictureEssenceCoding = true;
				ReadFile(hMXF, pictureEssenceCoding, 16, &nRead, NULL);
			break;

			default:
				SetFilePointer(hMXF, size, NULL, FILE_CURRENT);
		}
		
	}

	return TRUE;
}


unsigned int CDCIPictureEssenceDescriptor::CalcLength()
{
	length = genericPictureEssenceDescriptor.CalcLength() + // Generic Picture Essence Descriptor
			 4+4 + // Component Depth
			 4+4 + // Horizontal Subsampling
			 (bVerticalSubsampling?(4+4):0) + // Vertical Subsampling
			 (bColorSiting?(4+1):0) + // Color Siting
			 (bReversedByteOrder?(4+1):0) + // Reversed Byte Order
			 (bPaddingBits?(4+2):0) + // Padding Bits
			 (bAlphaSampleDepth?(4+4):0) + // Alpha Sample Depth
			 (bBlackRefLevel?(4+4):0) + // Black Ref Level
			 (bWhiteRefLevel?(4+4):0) + // White Ref Level
			 (bColorRange?(4+4):0); // Color Range

	return length;
}

unsigned int CDCIPictureEssenceDescriptor::Write(IWriter* pWriter, BOOL bWriteKL)
{
DWORD nWritten = 0;

	CalcLength();

	if(bWriteKL)
	{
		nWritten = WriteBuffer(key, 16, pWriter);
		nWritten += WriteBERLength(length, pWriter);
	}

	nWritten += genericPictureEssenceDescriptor.Write(pWriter, FALSE);

	nWritten += WriteNumber16(0x3301, pWriter);
	nWritten += WriteNumber16(4, pWriter);
	nWritten += WriteNumber32(componentDepth, pWriter);

	nWritten += WriteNumber16(0x3302, pWriter);
	nWritten += WriteNumber16(4, pWriter);
	nWritten += WriteNumber32(horizontalSubsampling, pWriter);

	if(bVerticalSubsampling)
	{
		nWritten += WriteNumber16(0x3308, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(verticalSubsampling, pWriter);
	}

	if(bColorSiting)
	{
		nWritten += WriteNumber16(0x3303, pWriter);
		nWritten += WriteNumber16(1, pWriter);
		nWritten += WriteNumber8(colorSiting, pWriter);
	}

	if(bReversedByteOrder)
	{
		nWritten += WriteNumber16(0x330b, pWriter);
		nWritten += WriteNumber16(1, pWriter);
		nWritten += WriteNumber8(reversedByteOrder, pWriter);
	}

	if(bPaddingBits)
	{
		nWritten += WriteNumber16(0x3307, pWriter);
		nWritten += WriteNumber16(2, pWriter);
		nWritten += WriteNumber16(paddingBits, pWriter);
	}

	if(bAlphaSampleDepth)
	{
		nWritten += WriteNumber16(0x3309, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(alphaSampleDepth, pWriter);
	}

	if(bBlackRefLevel)
	{
		nWritten += WriteNumber16(0x3304, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(blackRefLevel, pWriter);
	}

	if(bWhiteRefLevel)
	{
		nWritten += WriteNumber16(0x3305, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(whiteRefLevel, pWriter);
	}

	if(bColorRange)
	{
		nWritten += WriteNumber16(0x3306, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(colorRange, pWriter);
	}

	return nWritten;
}


BOOL CDCIPictureEssenceDescriptor::Read(HANDLE hMXF, DWORD chunkSize)
{
unsigned int localTag;
unsigned int size;
DWORD nRead;

	//genericPictureEssenceDescriptor.Read(hMXF);

	while(chunkSize > 0)
	{
		localTag = GetNumber16(hMXF);
		size = GetNumber16(hMXF);
		chunkSize -= 4;
		chunkSize -= size;

		switch(localTag)
		{
			// FILE DESCRIPTOR
			case 0x3c0a:
				ReadFile(hMXF, genericPictureEssenceDescriptor.fileDescriptor.instanceUID, 16, &nRead, NULL);
			break;
			case 0x0102:
				genericPictureEssenceDescriptor.fileDescriptor.bGenerationUID = true;
				ReadFile(hMXF, genericPictureEssenceDescriptor.fileDescriptor.generationUID, 16, &nRead, NULL);
			break;
			case 0x3006:
				genericPictureEssenceDescriptor.fileDescriptor.bLinkedTrackID = true;
				genericPictureEssenceDescriptor.fileDescriptor.linkedTrackID = GetNumber32(hMXF);
			break;
			case 0x3001:
				genericPictureEssenceDescriptor.fileDescriptor.sampleRate.num = GetNumber32(hMXF);
				genericPictureEssenceDescriptor.fileDescriptor.sampleRate.den = GetNumber32(hMXF);
			break;
			case 0x3002:
				genericPictureEssenceDescriptor.fileDescriptor.bContainerDuration = true;
				genericPictureEssenceDescriptor.fileDescriptor.containerDuration = GetNumber64(hMXF);
			break;
			case 0x3004:
				ReadFile(hMXF, genericPictureEssenceDescriptor.fileDescriptor.essenceContainer, 16, &nRead, NULL);
			break;
			case 0x3005:
				genericPictureEssenceDescriptor.fileDescriptor.bCodec = true;
				ReadFile(hMXF, genericPictureEssenceDescriptor.fileDescriptor.codec, 16, &nRead, NULL);
			break;
			case 0x2f01:
				genericPictureEssenceDescriptor.fileDescriptor.bLocators = true;
				genericPictureEssenceDescriptor.fileDescriptor.locators.Read(hMXF);
			break;

			// GENERIC PICTURE ESSENCE DESCRIPTOR
			case 0x3215:
				genericPictureEssenceDescriptor.bSignalStandard = true;
				genericPictureEssenceDescriptor.signalStandard = GetNumber8(hMXF);
			break;
			case 0x320c:
				genericPictureEssenceDescriptor.frameLayout = GetNumber8(hMXF);
			break;
			case 0x3203:
				genericPictureEssenceDescriptor.storedWidth = GetNumber32(hMXF);
			break;
			case 0x3202:
				genericPictureEssenceDescriptor.storedHeight = GetNumber32(hMXF);
			break;
			case 0x3216:
				genericPictureEssenceDescriptor.bStoredF2Offset = true;
				genericPictureEssenceDescriptor.storedF2Offset = (Int32)GetNumber32(hMXF);
			break;
			case 0x3205:
				genericPictureEssenceDescriptor.bSampledWidth = true;
				genericPictureEssenceDescriptor.sampledWidth = GetNumber32(hMXF);
			break;
			case 0x3204:
				genericPictureEssenceDescriptor.bSampledHeight = true;
				genericPictureEssenceDescriptor.sampledHeight = GetNumber32(hMXF);
			break;
			case 0x3206:
				genericPictureEssenceDescriptor.bSampledXOffset = true;
				genericPictureEssenceDescriptor.sampledXOffset = (Int32)GetNumber32(hMXF);
			break;
			case 0x3207:
				genericPictureEssenceDescriptor.bSampledYOffset = true;
				genericPictureEssenceDescriptor.sampledYOffset = (Int32)GetNumber32(hMXF);
			break;
			case 0x3208:
				genericPictureEssenceDescriptor.bDisplayHeight = true;
				genericPictureEssenceDescriptor.displayHeight = GetNumber32(hMXF);
			break;
			case 0x3209:
				genericPictureEssenceDescriptor.bDisplayWidth = true;
				genericPictureEssenceDescriptor.displayWidth = GetNumber32(hMXF);
			break;
			case 0x320a:
				genericPictureEssenceDescriptor.bDisplayXOffset = true;
				genericPictureEssenceDescriptor.displayXOffset = (Int32)GetNumber32(hMXF);
			break;
			case 0x320b:
				genericPictureEssenceDescriptor.bDisplayYOffset = true;
				genericPictureEssenceDescriptor.displayYOffset = (Int32)GetNumber32(hMXF);
			break;
			case 0x3217:
				genericPictureEssenceDescriptor.bDisplayF2Offset = true;
				genericPictureEssenceDescriptor.displayF2Offset = (Int32)GetNumber32(hMXF);
			break;
			case 0x320e:
				genericPictureEssenceDescriptor.aspectRatio.num = GetNumber32(hMXF);
				genericPictureEssenceDescriptor.aspectRatio.den = GetNumber32(hMXF);
			break;
			case 0x3218:
				genericPictureEssenceDescriptor.bActiveFormatDescriptor = true;
				genericPictureEssenceDescriptor.activeFormatDescriptor = GetNumber8(hMXF);
			break;
			case 0x320d:
				GetNumber32(hMXF);
				GetNumber32(hMXF);
				genericPictureEssenceDescriptor.videoLineMap[0] = GetNumber32(hMXF);
				genericPictureEssenceDescriptor.videoLineMap[1] = GetNumber32(hMXF);
			break;
			case 0x320f:
				genericPictureEssenceDescriptor.bAlphaTransparency = true;
				genericPictureEssenceDescriptor.alphaTransparency = GetNumber8(hMXF);
			break;
			case 0x3210:
				genericPictureEssenceDescriptor.bCaptureGamma = true;
				ReadFile(hMXF, genericPictureEssenceDescriptor.captureGamma, 16, &nRead, NULL);
			break;
			case 0x3211:
				genericPictureEssenceDescriptor.bImageAlignmentOffset = true;
				genericPictureEssenceDescriptor.imageAlignmentOffset = GetNumber32(hMXF);
			break;
			case 0x3213:
				genericPictureEssenceDescriptor.bImageStartOffset = true;
				genericPictureEssenceDescriptor.imageStartOffset = GetNumber32(hMXF);
			break;
			case 0x3214:
				genericPictureEssenceDescriptor.bImageEndOffset = true;
				genericPictureEssenceDescriptor.imageEndOffset = GetNumber32(hMXF);
			break;
			case 0x3212:
				genericPictureEssenceDescriptor.bFieldDominance = true;
				genericPictureEssenceDescriptor.fieldDominance = GetNumber8(hMXF);
			break;
			case 0x3201:
				genericPictureEssenceDescriptor.bPictureEssenceCoding = true;
				ReadFile(hMXF, genericPictureEssenceDescriptor.pictureEssenceCoding, 16, &nRead, NULL);
			break;

			// CDCI  PICTURE ESSENCE DESCRIPTOR
			case 0x3301:
				componentDepth = GetNumber32(hMXF);
			break;
			case 0x3302:
				horizontalSubsampling = GetNumber32(hMXF);
			break;
			case 0x3308:
				bVerticalSubsampling = true;
				verticalSubsampling = GetNumber32(hMXF);
			break;
			case 0x3303:
				bColorSiting = true;
				colorSiting = GetNumber8(hMXF);
			break;
			case 0x330b:
				bReversedByteOrder = true;
				reversedByteOrder = GetNumber8(hMXF);
			break;
			case 0x3307:
				bPaddingBits = true;
				paddingBits = GetNumber16(hMXF);
			break;
			case 0x3309:
				bAlphaSampleDepth = true;
				alphaSampleDepth = GetNumber32(hMXF);
			break;
			case 0x3304:
				bBlackRefLevel = true;
				blackRefLevel = GetNumber32(hMXF);
			break;
			case 0x3305:
				bWhiteRefLevel = true;
				whiteRefLevel = GetNumber32(hMXF);
			break;
			case 0x3306:
				bColorRange = true;
				colorRange = GetNumber32(hMXF);
			break;

			default:
				SetFilePointer(hMXF, size, NULL, FILE_CURRENT);
		}
	}


	return TRUE;
}


unsigned int GenericSoundEssenceDescriptor::CalcLength()
{
	length = fileDescriptor.CalcLength() +		  // File Descriptor
			 4+8 +								  // Sample Rate
			 (bLockedUnlocked?(4+1):0) +		  // Locked Unlocked
			 (bAudioRefLevel?(4+1):0) +			  // Audio Ref Level
			 (bElectroSpatialFormulation?(4+1):0) + // Electro Spatial Formulation
			 4+4 +								  // Channel Count
			 4+4 +								  // Quantization Bits
			 (bDialNorm?(4+1):0) +				  // Dial Norm
			 (bSoundEssenceCompression?(4+16):0); // Sound Essence Compression

	return length;
}


unsigned int GenericSoundEssenceDescriptor::Write(IWriter* pWriter, BOOL bWriteKL)
{
DWORD nWritten = 0;

	CalcLength();

	if(bWriteKL)
	{
		nWritten = WriteBuffer(key, 16, pWriter);
		nWritten += WriteBERLength(length, pWriter);
	}

	nWritten += fileDescriptor.Write(pWriter, FALSE);

	nWritten += WriteNumber16(0x3d03, pWriter);
	nWritten += WriteNumber16(8, pWriter);
	nWritten += WriteNumber32(audioSamplingRate.num, pWriter);
	nWritten += WriteNumber32(audioSamplingRate.den, pWriter);

	if(bLockedUnlocked)
	{
		nWritten += WriteNumber16(0x3d02, pWriter);
		nWritten += WriteNumber16(1, pWriter);
		nWritten += WriteNumber8(lockedUnlocked, pWriter);
	}

	if(bAudioRefLevel)
	{
		nWritten += WriteNumber16(0x3d04, pWriter);
		nWritten += WriteNumber16(1, pWriter);
		nWritten += WriteNumber8(audioRefLevel, pWriter);
	}

	if(bElectroSpatialFormulation)
	{
		nWritten += WriteNumber16(0x3d05, pWriter);
		nWritten += WriteNumber16(1, pWriter);
		nWritten += WriteNumber8(electroSpatialFormulation, pWriter);
	}

	nWritten += WriteNumber16(0x3d07, pWriter);
	nWritten += WriteNumber16(4, pWriter);
	nWritten += WriteNumber32(channelCount, pWriter);

	nWritten += WriteNumber16(0x3d01, pWriter);
	nWritten += WriteNumber16(4, pWriter);
	nWritten += WriteNumber32(quantizationBits, pWriter);

	if(bDialNorm)
	{
		nWritten += WriteNumber16(0x3d0c, pWriter);
		nWritten += WriteNumber16(1, pWriter);
		nWritten += WriteNumber8(dialNorm, pWriter);
	}

	if(bSoundEssenceCompression)
	{
		nWritten += WriteNumber16(0x3d06, pWriter);
		nWritten += WriteNumber16(16, pWriter);
		nWritten += WriteBuffer(soundEssenceCompression, 16, pWriter);
	}

	return nWritten;
}

BOOL GenericSoundEssenceDescriptor::Read(HANDLE hMXF, DWORD chunkSize)
{
unsigned int localTag;
unsigned int size;
DWORD nRead;

	//fileDescriptor.Read(hMXF);

	while(chunkSize > 0)
	{
		localTag = GetNumber16(hMXF);
		size = GetNumber16(hMXF);
		chunkSize -= 4;
		chunkSize -= size;

		switch(localTag)
		{
			// FILE DESCRIPTOR
			case 0x3c0a:
				ReadFile(hMXF, fileDescriptor.instanceUID, 16, &nRead, NULL);
			break;
			case 0x0102:
				fileDescriptor.bGenerationUID = true;
				ReadFile(hMXF, fileDescriptor.generationUID, 16, &nRead, NULL);
			break;
			case 0x3006:
				fileDescriptor.bLinkedTrackID = true;
				fileDescriptor.linkedTrackID = GetNumber32(hMXF);
			break;
			case 0x3001:
				fileDescriptor.sampleRate.num = GetNumber32(hMXF);
				fileDescriptor.sampleRate.den = GetNumber32(hMXF);
			break;
			case 0x3002:
				fileDescriptor.bContainerDuration = true;
				fileDescriptor.containerDuration = GetNumber64(hMXF);
			break;
			case 0x3004:
				ReadFile(hMXF, fileDescriptor.essenceContainer, 16, &nRead, NULL);
			break;
			case 0x3005:
				fileDescriptor.bCodec = true;
				ReadFile(hMXF, fileDescriptor.codec, 16, &nRead, NULL);
			break;
			case 0x2f01:
				fileDescriptor.bLocators = true;
				fileDescriptor.locators.Read(hMXF);
			break;

			// GENERIC SOUND ESSENCE DESCRIPTOR
			case 0x3d03:
				audioSamplingRate.num = GetNumber32(hMXF);
				audioSamplingRate.den = GetNumber32(hMXF);
			break;
			case 0x3d02:
				bLockedUnlocked = true;
				lockedUnlocked = GetNumber8(hMXF);
			break;
			case 0x3d04:
				bAudioRefLevel = true;
				audioRefLevel = (Int8)GetNumber8(hMXF);
			break;
			case 0x3d05:
				bElectroSpatialFormulation = true;
				electroSpatialFormulation = GetNumber8(hMXF);
			break;
			case 0x3d07:
				channelCount = GetNumber32(hMXF);
			break;
			case 0x3d01:
				quantizationBits = GetNumber32(hMXF);
			break;
			case 0x3d0c:
				bDialNorm = true;
				dialNorm = GetNumber8(hMXF);
			break;
			case 0x3d06:
				bSoundEssenceCompression = true;
				ReadFile(hMXF, soundEssenceCompression, 16, &nRead, NULL);
			break;

			default:
				SetFilePointer(hMXF, size, NULL, FILE_CURRENT);
		}
	}

	return TRUE;
}


unsigned int IndexTableSegment::CalcLength()
{
	length = 4+16 + // Instance ID
			 4+8 + // Index Edit Rate
			 4+8 + // Index Start Position
			 4+8 + // Index Duration
			 (bEditUnitByteCount?(4+4):0) + // Edit Unit Byte Count
			 (bIndexSID?(4+4):0) + // Index SID
			 4+4 + // BodySID
			 (bSliceCount?(4+1):0) + // Slice Count
			 (bPosTableCount?(4+1):0) + // Pos Table Count
			 (bDeltaEntryArray?(4+8+deltaEntryArray.NDE*deltaEntryArray.length):0) + // Delta Entry Array
			 (bIndexEntryArray?(4+8+indexEntryArray.NIE*indexEntryArray.length):0); // Index Entry Array

	return length;
}


unsigned int IndexTableSegment::Write(IWriter* pWriter)
{
unsigned int i, j;
DWORD nWritten = 0;

	CalcLength();	

	nWritten = WriteBuffer(key, 16, pWriter);
	nWritten += WriteBERLength(length, pWriter);

	nWritten += WriteNumber16(0x3c0a, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteBuffer(instanceID, 16, pWriter);

	nWritten += WriteNumber16(0x3f0b, pWriter);
	nWritten += WriteNumber16(8, pWriter);
	nWritten += WriteNumber32(indexEditRate.num, pWriter);
	nWritten += WriteNumber32(indexEditRate.den, pWriter);

	nWritten += WriteNumber16(0x3f0c, pWriter);
	nWritten += WriteNumber16(8, pWriter);
	nWritten += WriteNumber64(indexStartPosition, pWriter);

	nWritten += WriteNumber16(0x3f0d, pWriter);
	nWritten += WriteNumber16(8, pWriter);
	nWritten += WriteNumber64(indexDuration, pWriter);

	if(bEditUnitByteCount)
	{
		nWritten += WriteNumber16(0x3f05, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(editUnitByteCount, pWriter);
	}

	if(bIndexSID)
	{
		nWritten += WriteNumber16(0x3f06, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(indexSID, pWriter);
	}

	nWritten += WriteNumber16(0x3f07, pWriter);
	nWritten += WriteNumber16(4, pWriter);
	nWritten += WriteNumber32(bodySID, pWriter);

	if(bSliceCount)
	{
		nWritten += WriteNumber16(0x3f08, pWriter);
		nWritten += WriteNumber16(1, pWriter);
		nWritten += WriteNumber8(sliceCount, pWriter);
	}

	if(bPosTableCount)
	{
		nWritten += WriteNumber16(0x3f0e, pWriter);
		nWritten += WriteNumber16(1, pWriter);
		nWritten += WriteNumber8(posTableCount, pWriter);
	}

	if(bDeltaEntryArray)
	{
		nWritten += WriteNumber16(0x3f09, pWriter);
		nWritten += WriteNumber16(8+deltaEntryArray.NDE*deltaEntryArray.length, pWriter);
		nWritten += WriteNumber32(deltaEntryArray.NDE, pWriter);
		nWritten += WriteNumber32(deltaEntryArray.length, pWriter);
		for(i = 0; i < deltaEntryArray.NDE; i++)
		{
			nWritten += WriteNumber8(deltaEntryArray.pDeltaEntry[i].posTableIndex, pWriter);
			nWritten += WriteNumber8(deltaEntryArray.pDeltaEntry[i].slice, pWriter);
			nWritten += WriteNumber32(deltaEntryArray.pDeltaEntry[i].elementDelta, pWriter);
		}
	}

	if(bIndexEntryArray)
	{
		nWritten += WriteNumber16(0x3f0a, pWriter);
		nWritten += WriteNumber16(8+indexEntryArray.NIE*indexEntryArray.length, pWriter);
		nWritten += WriteNumber32(indexEntryArray.NIE, pWriter);
		nWritten += WriteNumber32(indexEntryArray.length, pWriter);
		for(i = 0; i < indexEntryArray.NIE; i++)
		{
			nWritten += WriteNumber8(indexEntryArray.pIndexEntry[i].temporalOffset, pWriter);
			nWritten += WriteNumber8(indexEntryArray.pIndexEntry[i].keyFrameOffset, pWriter);
			nWritten += WriteNumber8(indexEntryArray.pIndexEntry[i].flags, pWriter);
			nWritten += WriteNumber64(indexEntryArray.pIndexEntry[i].streamOffset, pWriter);
			for(j = 0; j < indexEntryArray.NSL; j++)
			{
				nWritten += WriteNumber32(indexEntryArray.pIndexEntry[i].pSliceOffset[j], pWriter);
			}
		}
	}

	return nWritten;
}

BOOL IndexTableSegment::Read(HANDLE hMXF, DWORD chunkSize)
{
unsigned short localTag;
unsigned short size;
DWORD nRead;

	while(chunkSize > 0)
	{
		localTag = GetNumber16(hMXF);
		size = GetNumber16(hMXF);
		chunkSize-=4; // 4 bytes (localtag + size)
		chunkSize-=size;

		switch(localTag)
		{
			case 0x3c0a:
				ReadFile(hMXF, instanceID, 16, &nRead, NULL);
			break;
			case 0x3f0b:
				indexEditRate.num = GetNumber32(hMXF);
				indexEditRate.den = GetNumber32(hMXF);
			break;
			case 0x3f0c:
				indexStartPosition = GetNumber64(hMXF);
			break;
			case 0x3f0d:
				indexDuration = GetNumber64(hMXF);
			break;
			case 0x3f05:
				bEditUnitByteCount = true;
				editUnitByteCount = GetNumber32(hMXF);
			break;
			case 0x3f06:
				bIndexSID = true;
				indexSID = GetNumber32(hMXF);
			break;
			case 0x3f07:
				bodySID = GetNumber32(hMXF);
			break;
			case 0x3f08:
				bSliceCount = true;
				sliceCount = GetNumber8(hMXF);
			break;
			case 0x3f0e:
				bPosTableCount = true;
				posTableCount = GetNumber8(hMXF);
			break;
			case 0x3f09:
				bDeltaEntryArray = true;
				deltaEntryArray.NDE = GetNumber32(hMXF);
				deltaEntryArray.length = GetNumber32(hMXF);
				deltaEntryArray.pDeltaEntry = new DeltaEntryArray::DeltaEntry[deltaEntryArray.NDE];
				for(unsigned int i = 0; i < deltaEntryArray.NDE; i++)
				{
					deltaEntryArray.pDeltaEntry[i].posTableIndex = (char)GetNumber8(hMXF);
					deltaEntryArray.pDeltaEntry[i].slice = GetNumber8(hMXF);
					deltaEntryArray.pDeltaEntry[i].elementDelta = GetNumber32(hMXF);
				}
			break;
			case 0x3f0a:
				bIndexEntryArray = true;
				indexEntryArray.NIE = GetNumber32(hMXF);
				indexEntryArray.length = GetNumber32(hMXF);
				indexEntryArray.pIndexEntry = new IndexEntryArray::IndexEntry[indexEntryArray.NIE];
				for(unsigned int i = 0; i < indexEntryArray.NIE; i++)
				{
					indexEntryArray.pIndexEntry[i].temporalOffset = (char)GetNumber8(hMXF);
					indexEntryArray.pIndexEntry[i].keyFrameOffset = (char)GetNumber8(hMXF);
					indexEntryArray.pIndexEntry[i].flags = GetNumber8(hMXF);
					indexEntryArray.pIndexEntry[i].streamOffset = GetNumber64(hMXF);

					if(bSliceCount && (sliceCount > 0))
					{
						indexEntryArray.pIndexEntry[i].pSliceOffset = new unsigned int[sliceCount];
						for(unsigned int j=0; j < sliceCount; j++)
						{
							indexEntryArray.pIndexEntry[i].pSliceOffset[j] = GetNumber32(hMXF);							
						}
					}

					if(bPosTableCount && (posTableCount > 0))
					{
						indexEntryArray.pIndexEntry[i].pPosTable = new Rational[posTableCount];
						for(unsigned int j=0; j < sliceCount; j++)
						{
							indexEntryArray.pIndexEntry[i].pPosTable[j].num = GetNumber32(hMXF);
							indexEntryArray.pIndexEntry[i].pPosTable[j].den = GetNumber32(hMXF);
						}
					}
				}
			break;

			// Pula (size) bytes
			default:
				SetFilePointer(hMXF, size, NULL, FILE_CURRENT);
			break;
		}
	}


	return TRUE;
}


unsigned int SystemMetadataPack::CalcLength()
{
	length = 1 + // System Metadata Bitmap
			 1 + // Content Package Rate
			 1 + // Content Package Type
			 2 + // Channel Handle
			 2 + // Continuity Count
			 16 + // Universal Label
			 1+16 + // Creation Date Time Stamp
			 1+16; // User Date Time Stamp
			 

	return length;			 
}


unsigned int SystemMetadataPack::Write(IWriter* pWriter)
{
DWORD nWritten;

	CalcLength();

	nWritten = WriteBuffer(key, 16, pWriter);
	nWritten += WriteBERLength(length, pWriter);

	nWritten += WriteNumber8(systemMetadataBitmap, pWriter);
	nWritten += WriteNumber8(contentPackageRate, pWriter);
	nWritten += WriteNumber8(contentPackageType, pWriter);
	nWritten += WriteNumber16(channelHandle, pWriter);
	nWritten += WriteNumber16(continuityCount, pWriter);
	nWritten += WriteBuffer(universalLabel, 16, pWriter);

	nWritten += WriteNumber8(creationDateTimeStamp.type, pWriter);
	nWritten += WriteBuffer(creationDateTimeStamp.data, 16, pWriter);

	nWritten += WriteNumber8(userDateTimeStamp.type, pWriter);
	nWritten += WriteBuffer(userDateTimeStamp.data, 16, pWriter);

	return nWritten;
}


BOOL SystemMetadataPack::Read(HANDLE hMXF)
{
DWORD nRead;

	systemMetadataBitmap = GetNumber8(hMXF);
	contentPackageRate = GetNumber8(hMXF);
	contentPackageType = GetNumber8(hMXF);
	channelHandle = GetNumber16(hMXF);
	continuityCount = GetNumber16(hMXF);
	ReadFile(hMXF, universalLabel, 16, &nRead, NULL);
	ReadFile(hMXF, &creationDateTimeStamp, sizeof(DateTimeStamp), &nRead, NULL);
	ReadFile(hMXF, &userDateTimeStamp, sizeof(DateTimeStamp), &nRead, NULL);

	return TRUE;
}

unsigned int MPEG2VideoDescriptor::CalcLength()
{
	length = cdciPictureEssenceDescriptor.CalcLength() + 
			(bSingleSequence?(4+1):0)+ // SingleSequence
			(bConstantBFrames?(4+1):0)+ // Constant B Frames
			(bCodedContentType?(4+1):0)+ // Coded Content Type
			(bLowDelay?(4+1):0)+ // Low Delay
			(bClosedGOP?(4+1):0)+ // Closed GOP
			(bIdenticalGOP?(4+1):0)+ // Identical GOP
			(bMaxGOP?(4+2):0)+ // Max GOP
			(bBPictureCount?(4+2):0)+ // B Picture Count
			(bBitRate?(4+4):0)+ // Bit Rate
			(bProfileAndLevel?(4+1):0); // Profile and Level
		

	return length;
}

unsigned int MPEG2VideoDescriptor::Write(IWriter* pWriter, PrimerPack* pPrimerPack)
{
DWORD nWritten;
unsigned short localTag;

	CalcLength();

	nWritten = WriteBuffer(key, 16, pWriter);
	nWritten += WriteBERLength(length, pWriter);

	nWritten += cdciPictureEssenceDescriptor.Write(pWriter, FALSE);

	if(bSingleSequence)
	{
		localTag = pPrimerPack->FindLocalTag(0x0401060201020000);
		nWritten += WriteNumber16(localTag, pWriter);
		nWritten += WriteNumber16(1, pWriter);
		nWritten += WriteNumber8(singleSequence, pWriter);
	}

	if(bConstantBFrames)
	{
		localTag = pPrimerPack->FindLocalTag(0x0401060201030000);
		nWritten += WriteNumber16(localTag, pWriter);
		nWritten += WriteNumber16(1, pWriter);
		nWritten += WriteNumber8(constantBFrames, pWriter);
	}


	if(bCodedContentType)
	{
		localTag = pPrimerPack->FindLocalTag(0x0401060201040000);
		nWritten += WriteNumber16(localTag, pWriter);
		nWritten += WriteNumber16(1, pWriter);
		nWritten += WriteNumber8(codedContentType, pWriter);
	}

	if(bLowDelay)
	{
		localTag = pPrimerPack->FindLocalTag(0x0401060201050000);
		nWritten += WriteNumber16(localTag, pWriter);
		nWritten += WriteNumber16(1, pWriter);
		nWritten += WriteNumber8(lowDelay, pWriter);
	}

	if(bClosedGOP)
	{
		localTag = pPrimerPack->FindLocalTag(0x0401060201060000);
		nWritten += WriteNumber16(localTag, pWriter);
		nWritten += WriteNumber16(1, pWriter);
		nWritten += WriteNumber8(closedGOP, pWriter);
	}

	if(bIdenticalGOP)
	{
		localTag = pPrimerPack->FindLocalTag(0x0401060201070000);
		nWritten += WriteNumber16(localTag, pWriter);
		nWritten += WriteNumber16(1, pWriter);
		nWritten += WriteNumber8(identicalGOP, pWriter);
	}


	if(bMaxGOP)
	{
		localTag = pPrimerPack->FindLocalTag(0x0401060201080000);
		nWritten += WriteNumber16(localTag, pWriter);
		nWritten += WriteNumber16(2, pWriter);
		nWritten += WriteNumber16(maxGOP, pWriter);
	}

	if(bBPictureCount)
	{
		localTag = pPrimerPack->FindLocalTag(0x0401060201090000);
		nWritten += WriteNumber16(localTag, pWriter);
		nWritten += WriteNumber16(2, pWriter);
		nWritten += WriteNumber16(bPictureCount, pWriter);
	}

	if(bBitRate)
	{
		localTag = pPrimerPack->FindLocalTag(0x04010602010B0000);
		nWritten += WriteNumber16(localTag, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(bitRate, pWriter);
	}

	if(bProfileAndLevel)
	{
		localTag = pPrimerPack->FindLocalTag(0x04010602010A0000);
		nWritten += WriteNumber16(localTag, pWriter);
		nWritten += WriteNumber16(1, pWriter);
		nWritten += WriteNumber8(profileAndLevel, pWriter);
	}

	return nWritten;
}


BOOL MPEG2VideoDescriptor::Read(HANDLE hMXF, PrimerPack* pPrimerPack, DWORD chunkSize)
{
unsigned short localTag;
UInt64 itemDesignator;
unsigned short size;
DWORD nRead;

	while(chunkSize > 0)
	{
		localTag = GetNumber16(hMXF);
		size = GetNumber16(hMXF);
		chunkSize-=4; // 4 bytes (localtag + size)
		chunkSize-=size;

		switch(localTag)
		{
			// FILE DESCRIPTOR
			case 0x3c0a:
				ReadFile(hMXF, cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.fileDescriptor.instanceUID, 16, &nRead, NULL);
			break;
			case 0x0102:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.fileDescriptor.bGenerationUID = true;
				ReadFile(hMXF, cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.fileDescriptor.generationUID, 16, &nRead, NULL);
			break;
			case 0x3006:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.fileDescriptor.bLinkedTrackID = true;
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.fileDescriptor.linkedTrackID = GetNumber32(hMXF);
			break;
			case 0x3001:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.fileDescriptor.sampleRate.num = GetNumber32(hMXF);
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.fileDescriptor.sampleRate.den = GetNumber32(hMXF);
			break;
			case 0x3002:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.fileDescriptor.bContainerDuration = true;
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.fileDescriptor.containerDuration = GetNumber64(hMXF);
			break;
			case 0x3004:
				ReadFile(hMXF, cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.fileDescriptor.essenceContainer, 16, &nRead, NULL);
			break;
			case 0x3005:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.fileDescriptor.bCodec = true;
				ReadFile(hMXF, cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.fileDescriptor.codec, 16, &nRead, NULL);
			break;
			case 0x2f01:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.fileDescriptor.bLocators = true;
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.fileDescriptor.locators.Read(hMXF);
			break;

			// GENERIC PICTURE ESSENCE DESCRIPTOR
			case 0x3215:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.bSignalStandard = true;
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.signalStandard = GetNumber8(hMXF);
			break;
			case 0x320c:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.frameLayout = GetNumber8(hMXF);
			break;
			case 0x3203:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.storedWidth = GetNumber32(hMXF);
			break;
			case 0x3202:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.storedHeight = GetNumber32(hMXF);
			break;
			case 0x3216:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.bStoredF2Offset = true;
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.storedF2Offset = (Int32)GetNumber32(hMXF);
			break;
			case 0x3205:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.bSampledWidth = true;
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.sampledWidth = GetNumber32(hMXF);
			break;
			case 0x3204:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.bSampledHeight = true;
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.sampledHeight = GetNumber32(hMXF);
			break;
			case 0x3206:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.bSampledXOffset = true;
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.sampledXOffset = (Int32)GetNumber32(hMXF);
			break;
			case 0x3207:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.bSampledYOffset = true;
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.sampledYOffset = (Int32)GetNumber32(hMXF);
			break;
			case 0x3208:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.bDisplayHeight = true;
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.displayHeight = GetNumber32(hMXF);
			break;
			case 0x3209:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.bDisplayWidth = true;
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.displayWidth = GetNumber32(hMXF);
			break;
			case 0x320a:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.bDisplayXOffset = true;
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.displayXOffset = (Int32)GetNumber32(hMXF);
			break;
			case 0x320b:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.bDisplayYOffset = true;
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.displayYOffset = (Int32)GetNumber32(hMXF);
			break;
			case 0x3217:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.bDisplayF2Offset = true;
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.displayF2Offset = (Int32)GetNumber32(hMXF);
			break;
			case 0x320e:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.aspectRatio.num = GetNumber32(hMXF);
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.aspectRatio.den = GetNumber32(hMXF);
			break;
			case 0x3218:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.bActiveFormatDescriptor = true;
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.activeFormatDescriptor = GetNumber8(hMXF);
			break;
			case 0x320d:
				GetNumber32(hMXF);
				GetNumber32(hMXF);
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.videoLineMap[0] = GetNumber32(hMXF);
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.videoLineMap[1] = GetNumber32(hMXF);
			break;
			case 0x320f:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.bAlphaTransparency = true;
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.alphaTransparency = GetNumber8(hMXF);
			break;
			case 0x3210:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.bCaptureGamma = true;
				ReadFile(hMXF, cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.captureGamma, 16, &nRead, NULL);
			break;
			case 0x3211:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.bImageAlignmentOffset = true;
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.imageAlignmentOffset = GetNumber32(hMXF);
			break;
			case 0x3213:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.bImageStartOffset = true;
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.imageStartOffset = GetNumber32(hMXF);
			break;
			case 0x3214:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.bImageEndOffset = true;
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.imageEndOffset = GetNumber32(hMXF);
			break;
			case 0x3212:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.bFieldDominance = true;
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.fieldDominance = GetNumber8(hMXF);
			break;
			case 0x3201:
				cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.bPictureEssenceCoding = true;
				ReadFile(hMXF, cdciPictureEssenceDescriptor.genericPictureEssenceDescriptor.pictureEssenceCoding, 16, &nRead, NULL);
			break;

			// CDCI  PICTURE ESSENCE DESCRIPTOR
			case 0x3301:
				cdciPictureEssenceDescriptor.componentDepth = GetNumber32(hMXF);
			break;
			case 0x3302:
				cdciPictureEssenceDescriptor.horizontalSubsampling = GetNumber32(hMXF);
			break;
			case 0x3308:
				cdciPictureEssenceDescriptor.bVerticalSubsampling = true;
				cdciPictureEssenceDescriptor.verticalSubsampling = GetNumber32(hMXF);
			break;
			case 0x3303:
				cdciPictureEssenceDescriptor.bColorSiting = true;
				cdciPictureEssenceDescriptor.colorSiting = GetNumber8(hMXF);
			break;
			case 0x330b:
				cdciPictureEssenceDescriptor.bReversedByteOrder = true;
				cdciPictureEssenceDescriptor.reversedByteOrder = GetNumber8(hMXF);
			break;
			case 0x3307:
				cdciPictureEssenceDescriptor.bPaddingBits = true;
				cdciPictureEssenceDescriptor.paddingBits = GetNumber16(hMXF);
			break;
			case 0x3309:
				cdciPictureEssenceDescriptor.bAlphaSampleDepth = true;
				cdciPictureEssenceDescriptor.alphaSampleDepth = GetNumber32(hMXF);
			break;
			case 0x3304:
				cdciPictureEssenceDescriptor.bBlackRefLevel = true;
				cdciPictureEssenceDescriptor.blackRefLevel = GetNumber32(hMXF);
			break;
			case 0x3305:
				cdciPictureEssenceDescriptor.bWhiteRefLevel = true;
				cdciPictureEssenceDescriptor.whiteRefLevel = GetNumber32(hMXF);
			break;
			case 0x3306:
				cdciPictureEssenceDescriptor.bColorRange = true;
				cdciPictureEssenceDescriptor.colorRange = GetNumber32(hMXF);
			break;


			default:
			itemDesignator = pPrimerPack->FindItemDesignator(localTag);
			switch(itemDesignator)
			{
				case 0x0401060201020000:
					bSingleSequence = true;
					singleSequence = GetNumber8(hMXF);
				break;
				case 0x0401060201030000:
					bConstantBFrames = true;
					constantBFrames = GetNumber8(hMXF);
				break;
				case 0x0401060201040000:
					bCodedContentType = true;
					codedContentType = GetNumber8(hMXF);
				break;
				case 0x0401060201050000:
					bLowDelay = true;
					lowDelay = GetNumber8(hMXF);
				break;
				case 0x0401060201060000:
					bClosedGOP = true;
					closedGOP = GetNumber8(hMXF);
				break;
				case 0x0401060201070000:
					bIdenticalGOP = true;
					identicalGOP = GetNumber8(hMXF);
				break;
				case 0x0401060201080000:
					bMaxGOP = true;
					maxGOP = GetNumber16(hMXF);
				break;
				case 0x0401060201090000:
					bBPictureCount = true;
					bPictureCount = GetNumber16(hMXF);
				break;
				case 0x04010602010B0000:
					bBitRate = true;
					bitRate = GetNumber32(hMXF);
				break;
				case 0x04010602010A0000:
					bProfileAndLevel = true;
					profileAndLevel = GetNumber8(hMXF);
				break;
				default:
					SetFilePointer(hMXF, size, NULL, FILE_CURRENT);
			}
		}
	}

	return TRUE;
}


void UInt8Array::Read(HANDLE hMXF)
{
unsigned int i;

	num = GetNumber32(hMXF);
	size = GetNumber32(hMXF);
	ASSERT(size == 1);

	Alloc(num);

	for(i = 0; i < num; i++)
	{
		pElements[i] = GetNumber8(hMXF);
	}
}

BOOL WaveAudioEssenceDescriptor::Read(HANDLE hMXF, DWORD chunkSize)
{
unsigned short localTag;
unsigned short size;
DWORD nRead;

	while(chunkSize > 0)
	{
		localTag = GetNumber16(hMXF);
		size = GetNumber16(hMXF);
		chunkSize-=4; // 4 bytes (localtag + size)
		chunkSize-=size;

		switch(localTag)
		{
			// FILE DESCRIPTOR
			case 0x3c0a:
				ReadFile(hMXF, genericSoundEssenceDescriptor.fileDescriptor.instanceUID, 16, &nRead, NULL);
			break;
			case 0x0102:
				genericSoundEssenceDescriptor.fileDescriptor.bGenerationUID = true;
				ReadFile(hMXF, genericSoundEssenceDescriptor.fileDescriptor.generationUID, 16, &nRead, NULL);
			break;
			case 0x3006:
				genericSoundEssenceDescriptor.fileDescriptor.bLinkedTrackID = true;
				genericSoundEssenceDescriptor.fileDescriptor.linkedTrackID = GetNumber32(hMXF);
			break;
			case 0x3001:
				genericSoundEssenceDescriptor.fileDescriptor.sampleRate.num = GetNumber32(hMXF);
				genericSoundEssenceDescriptor.fileDescriptor.sampleRate.den = GetNumber32(hMXF);
			break;
			case 0x3002:
				genericSoundEssenceDescriptor.fileDescriptor.bContainerDuration = true;
				genericSoundEssenceDescriptor.fileDescriptor.containerDuration = GetNumber64(hMXF);
			break;
			case 0x3004:
				ReadFile(hMXF, genericSoundEssenceDescriptor.fileDescriptor.essenceContainer, 16, &nRead, NULL);
			break;
			case 0x3005:
				genericSoundEssenceDescriptor.fileDescriptor.bCodec = true;
				ReadFile(hMXF, genericSoundEssenceDescriptor.fileDescriptor.codec, 16, &nRead, NULL);
			break;
			case 0x2f01:
				genericSoundEssenceDescriptor.fileDescriptor.bLocators = true;
				genericSoundEssenceDescriptor.fileDescriptor.locators.Read(hMXF);
			break;

			// GENERIC SOUND ESSENCE DESCRIPTOR
			case 0x3d03:
				genericSoundEssenceDescriptor.audioSamplingRate.num = GetNumber32(hMXF);
				genericSoundEssenceDescriptor.audioSamplingRate.den = GetNumber32(hMXF);
			break;
			case 0x3d02:
				genericSoundEssenceDescriptor.bLockedUnlocked = true;
				genericSoundEssenceDescriptor.lockedUnlocked = GetNumber8(hMXF);
			break;
			case 0x3d04:
				genericSoundEssenceDescriptor.bAudioRefLevel = true;
				genericSoundEssenceDescriptor.audioRefLevel = (Int8)GetNumber8(hMXF);
			break;
			case 0x3d05:
				genericSoundEssenceDescriptor.bElectroSpatialFormulation = true;
				genericSoundEssenceDescriptor.electroSpatialFormulation = GetNumber8(hMXF);
			break;
			case 0x3d07:
				genericSoundEssenceDescriptor.channelCount = GetNumber32(hMXF);
			break;
			case 0x3d01:
				genericSoundEssenceDescriptor.quantizationBits = GetNumber32(hMXF);
			break;
			case 0x3d0c:
				genericSoundEssenceDescriptor.bDialNorm = true;
				genericSoundEssenceDescriptor.dialNorm = GetNumber8(hMXF);
			break;
			case 0x3d06:
				genericSoundEssenceDescriptor.bSoundEssenceCompression = true;
				ReadFile(hMXF, genericSoundEssenceDescriptor.soundEssenceCompression, 16, &nRead, NULL);
			break;


			// WAVE AUDIO ESSENCE DESCRIPTOR
			case 0x3d0a:
				blockAlign = GetNumber16(hMXF);
			break;
			case 0x3d0b:
				bSequenceOffset = true;
				sequenceOffset = GetNumber8(hMXF);
			break;
			case 0x3d09:
				avgBps = GetNumber32(hMXF);
			break;
			case 0x3d32:
				bChannelAssignment = true;
				ReadFile(hMXF, channelAssignment, 16, &nRead, NULL);
			break;
			case 0x3d29:
				bPeakEnvelopeVersion = true;
				peakEnvelopeVersion = GetNumber32(hMXF);
			break;
			case 0x3d2a:
				bPeakEnvelopeFormat = true;
				peakEnvelopeFormat = GetNumber32(hMXF);
			break;
			case 0x3d2b:
				bPointsPerPeakValue = true;
				pointsPerPeakValue = GetNumber32(hMXF);
			break;
			case 0x3d2c:
				bPeakEnvelopeBlockSize = true;
				peakEnvelopeBlockSize = GetNumber32(hMXF);
			break;
			case 0x3d2d:
				bPeakChannels = true;
				peakChannels = GetNumber32(hMXF);
			break;
			case 0x3d2e:
				bPeakFrames = true;
				peakFrames = GetNumber32(hMXF);
			break;
			case 0x3d2f:
				bPeakOfPeaksPosition = true;
				peakOfPeaksPosition = GetNumber64(hMXF);
			break;
			case 0x3d30:
				bPeakEnvelopeTimestamp = true;
				peakEnvelopeTimestamp.i64Data = GetNumber64(hMXF);
			break;
			case 0x3d31:
				bPeakEnvelopeData = true;
				// ????
				// ????
			break;
			default:				
				SetFilePointer(hMXF, size, NULL, FILE_CURRENT);
		}
	}

	return TRUE;
}


unsigned int WaveAudioEssenceDescriptor::CalcLength()
{
	length = genericSoundEssenceDescriptor.CalcLength() +
			4+2 +								// block align
			(bSequenceOffset?(4+1):0) +			// Sequence offset
			4+4 +								// Avg Bps
			(bChannelAssignment?(4+16):0) +		// Channel Assignment
			(bPeakEnvelopeVersion?(4+4):0) +	// Peak Envelope Version
			(bPeakEnvelopeFormat?(4+4):0) +		// Peak Envelope Format
			(bPointsPerPeakValue?(4+4):0) +		// Points Per Peak Value
			(bPeakEnvelopeBlockSize?(4+4):0) +	// Peak Envelope Block Size
			(bPeakChannels?(4+4):0) +			// Peak Channels
			(bPeakFrames?(4+4):0) +				// Peak Frames
			(bPeakOfPeaksPosition?(4+8):0) +	// Peak of Peaks Position
			(bPeakEnvelopeTimestamp?(4+8):0) +	// Peak Envelope TimeStamp
			0; // Peak Envelope Data (??????)

	return length;
}


unsigned int WaveAudioEssenceDescriptor::Write(IWriter* pWriter, BOOL bWriteKL)
{
DWORD nWritten = 0;

	CalcLength();

	if(bWriteKL)
	{
		nWritten = WriteBuffer(key, 16, pWriter);
		nWritten += WriteBERLength(length, pWriter);
	}

	nWritten += genericSoundEssenceDescriptor.Write(pWriter, FALSE);

	nWritten += WriteNumber16(0x3d0a, pWriter);
	nWritten += WriteNumber16(2, pWriter);
	nWritten += WriteNumber16(blockAlign, pWriter);

	if(bSequenceOffset)
	{
		nWritten += WriteNumber16(0x3d0b, pWriter);
		nWritten += WriteNumber16(1, pWriter);
		nWritten += WriteNumber8(sequenceOffset, pWriter);
	}

	nWritten += WriteNumber16(0x3d09, pWriter);
	nWritten += WriteNumber16(4, pWriter);
	nWritten += WriteNumber32(avgBps, pWriter);

	if(bChannelAssignment)
	{
		nWritten += WriteNumber16(0x3d32, pWriter);
		nWritten += WriteNumber16(16, pWriter);
		nWritten += WriteBuffer(channelAssignment, 16, pWriter);
	}

	if(bPeakEnvelopeVersion)
	{
		nWritten += WriteNumber16(0x3d29, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(peakEnvelopeVersion, pWriter);
	}

	if(bPeakEnvelopeFormat)
	{
		nWritten += WriteNumber16(0x3d2a, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(peakEnvelopeFormat, pWriter);
	}

	if(bPointsPerPeakValue)
	{
		nWritten += WriteNumber16(0x3d2b, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(pointsPerPeakValue, pWriter);
	}

	if(bPeakEnvelopeBlockSize)
	{
		nWritten += WriteNumber16(0x3d2c, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(peakEnvelopeBlockSize, pWriter);
	}

	if(bPeakChannels)
	{
		nWritten += WriteNumber16(0x3d2d, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(peakChannels, pWriter);
	}

	if(bPeakFrames)
	{
		nWritten += WriteNumber16(0x3d2e, pWriter);
		nWritten += WriteNumber16(4, pWriter);
		nWritten += WriteNumber32(peakFrames, pWriter);
	}

	if(bPeakOfPeaksPosition)
	{
		nWritten += WriteNumber16(0x3d2f, pWriter);
		nWritten += WriteNumber16(8, pWriter);
		nWritten += WriteNumber64(peakOfPeaksPosition, pWriter);
	}


	if(bPeakEnvelopeTimestamp)
	{
		nWritten += WriteNumber16(0x3d30, pWriter);
		nWritten += WriteNumber16(8, pWriter);
		nWritten += WriteNumber64(peakEnvelopeTimestamp.i64Data, pWriter);
	}

	if(bPeakEnvelopeData)
	{
		// ????????
	}

	return nWritten;
}



BOOL AES3AudioEssenceDescriptor::Read(HANDLE hMXF, DWORD chunkSize)
{
unsigned short localTag;
unsigned short size;
DWORD nRead;

	while(chunkSize > 0)
	{
		localTag = GetNumber16(hMXF);
		size = GetNumber16(hMXF);
		chunkSize-=4; // 4 bytes (localtag + size)
		chunkSize-=size;

		switch(localTag)
		{
			// FILE DESCRIPTOR
			case 0x3c0a:
				ReadFile(hMXF, waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.fileDescriptor.instanceUID, 16, &nRead, NULL);
			break;
			case 0x0102:
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.fileDescriptor.bGenerationUID = true;
				ReadFile(hMXF, waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.fileDescriptor.generationUID, 16, &nRead, NULL);
			break;
			case 0x3006:
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.fileDescriptor.bLinkedTrackID = true;
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.fileDescriptor.linkedTrackID = GetNumber32(hMXF);
			break;
			case 0x3001:
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.fileDescriptor.sampleRate.num = GetNumber32(hMXF);
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.fileDescriptor.sampleRate.den = GetNumber32(hMXF);
			break;
			case 0x3002:
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.fileDescriptor.bContainerDuration = true;
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.fileDescriptor.containerDuration = GetNumber64(hMXF);
			break;
			case 0x3004:
				ReadFile(hMXF, waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.fileDescriptor.essenceContainer, 16, &nRead, NULL);
			break;
			case 0x3005:
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.fileDescriptor.bCodec = true;
				ReadFile(hMXF, waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.fileDescriptor.codec, 16, &nRead, NULL);
			break;
			case 0x2f01:
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.fileDescriptor.bLocators = true;
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.fileDescriptor.locators.Read(hMXF);
			break;

			// GENERIC SOUND ESSENCE DESCRIPTOR
			case 0x3d03:
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.audioSamplingRate.num = GetNumber32(hMXF);
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.audioSamplingRate.den = GetNumber32(hMXF);
			break;
			case 0x3d02:
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.bLockedUnlocked = true;
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.lockedUnlocked = GetNumber8(hMXF);
			break;
			case 0x3d04:
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.bAudioRefLevel = true;
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.audioRefLevel = (Int8)GetNumber8(hMXF);
			break;
			case 0x3d05:
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.bElectroSpatialFormulation = true;
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.electroSpatialFormulation = GetNumber8(hMXF);
			break;
			case 0x3d07:
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.channelCount = GetNumber32(hMXF);
			break;
			case 0x3d01:
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.quantizationBits = GetNumber32(hMXF);
			break;
			case 0x3d0c:
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.bDialNorm = true;
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.dialNorm = GetNumber8(hMXF);
			break;
			case 0x3d06:
				waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.bSoundEssenceCompression = true;
				ReadFile(hMXF, waveAudioEssenceDescriptor.genericSoundEssenceDescriptor.soundEssenceCompression, 16, &nRead, NULL);
			break;


			// WAVE AUDIO ESSENCE DESCRIPTOR
			case 0x3d0a:
				waveAudioEssenceDescriptor.blockAlign = GetNumber16(hMXF);
			break;
			case 0x3d0b:
				waveAudioEssenceDescriptor.bSequenceOffset = true;
				waveAudioEssenceDescriptor.sequenceOffset = GetNumber8(hMXF);
			break;
			case 0x3d09:
				waveAudioEssenceDescriptor.avgBps = GetNumber32(hMXF);
			break;
			case 0x3d32:
				waveAudioEssenceDescriptor.bChannelAssignment = true;
				ReadFile(hMXF, waveAudioEssenceDescriptor.channelAssignment, 16, &nRead, NULL);
			break;
			case 0x3d29:
				waveAudioEssenceDescriptor.bPeakEnvelopeVersion = true;
				waveAudioEssenceDescriptor.peakEnvelopeVersion = GetNumber32(hMXF);
			break;
			case 0x3d2a:
				waveAudioEssenceDescriptor.bPeakEnvelopeFormat = true;
				waveAudioEssenceDescriptor.peakEnvelopeFormat = GetNumber32(hMXF);
			break;
			case 0x3d2b:
				waveAudioEssenceDescriptor.bPointsPerPeakValue = true;
				waveAudioEssenceDescriptor.pointsPerPeakValue = GetNumber32(hMXF);
			break;
			case 0x3d2c:
				waveAudioEssenceDescriptor.bPeakEnvelopeBlockSize = true;
				waveAudioEssenceDescriptor.peakEnvelopeBlockSize = GetNumber32(hMXF);
			break;
			case 0x3d2d:
				waveAudioEssenceDescriptor.bPeakChannels = true;
				waveAudioEssenceDescriptor.peakChannels = GetNumber32(hMXF);
			break;
			case 0x3d2e:
				waveAudioEssenceDescriptor.bPeakFrames = true;
				waveAudioEssenceDescriptor.peakFrames = GetNumber32(hMXF);
			break;
			case 0x3d2f:
				waveAudioEssenceDescriptor.bPeakOfPeaksPosition = true;
				waveAudioEssenceDescriptor.peakOfPeaksPosition = GetNumber64(hMXF);
			break;
			case 0x3d30:
				waveAudioEssenceDescriptor.bPeakEnvelopeTimestamp = true;
				waveAudioEssenceDescriptor.peakEnvelopeTimestamp.i64Data = GetNumber64(hMXF);
			break;
			case 0x3d31:
				waveAudioEssenceDescriptor.bPeakEnvelopeData = true;
				// ????
				// ????
			break;

			// AES3 AUDIO ESSENCE DESCRIPTOR
			case 0x3d0d:
				bEmphasis = true;
				emphasis = GetNumber8(hMXF);
			break;
			case 0x3d0f:
				bBlockStartOffset = true;
				blockStartOffset = GetNumber16(hMXF);
			break;
			case 0x3d08:
				bAuxBitsMode = true;
				auxBitsMode = GetNumber8(hMXF);
			break;
			case 0x3d10:
				bChannelStatusMode = true;
				channelStatusMode.Read(hMXF);
			break;
			case 0x3d11:
				bFixedChannelStatusData = true;
				fixedChannelStatusData.Read(hMXF);
			break;
			case 0x3d12:
				bUserDataMode = true;
				userDataMode.Read(hMXF);
			break;
			case 0x3d13:
				bFixedUserData = true;
				fixedUserData.Read(hMXF);
			break;
			default:
				SetFilePointer(hMXF, size, NULL, FILE_CURRENT);
		}
	}
	return TRUE;
}

unsigned int AES3AudioEssenceDescriptor::CalcLength()
{
	length = waveAudioEssenceDescriptor.CalcLength() +
			(bEmphasis?(4+1):0) +																		// Emphasis
			(bBlockStartOffset?(4+2):0) +																// BlockStartOffset
			(bAuxBitsMode?(4+1):0) +																	// AuxBitsMode
			(bChannelStatusMode?(4+8+channelStatusMode.num*channelStatusMode.size):0) +					// ChannelStatusMode
			(bFixedChannelStatusData?(4+8+fixedChannelStatusData.size*fixedChannelStatusData.num):0) +	// FixedChannelStatusDAta
			(bUserDataMode?(4+8+userDataMode.num):0) +													// UserDataMode
			(bFixedUserData?(4+8+fixedUserData.size*fixedUserData.num):0);								// FixedUserData

	return length;
}

unsigned int AES3AudioEssenceDescriptor::Write(IWriter* pWriter)
{
DWORD nWritten;

	CalcLength();

	nWritten = WriteBuffer(key, 16, pWriter);
	nWritten += WriteBERLength(length, pWriter);

	nWritten += waveAudioEssenceDescriptor.Write(pWriter, FALSE);

	if(bEmphasis)
	{
		nWritten += WriteNumber16(0x3d0d, pWriter);
		nWritten += WriteNumber16(1, pWriter);
		nWritten += WriteNumber8(emphasis, pWriter);
	}

	if(bBlockStartOffset)
	{
		nWritten += WriteNumber16(0x3d0f, pWriter);
		nWritten += WriteNumber16(2, pWriter);
		nWritten += WriteNumber16(blockStartOffset, pWriter);
	}

	if(bAuxBitsMode)
	{
		nWritten += WriteNumber16(0x3d08, pWriter);
		nWritten += WriteNumber16(1, pWriter);
		nWritten += WriteNumber8(auxBitsMode, pWriter);
	}
	if(bChannelStatusMode)
	{
		nWritten += WriteNumber16(0x3d10, pWriter);
		nWritten += WriteNumber16(8+channelStatusMode.num*channelStatusMode.size, pWriter);
		nWritten += WriteNumber32(channelStatusMode.num, pWriter);
		nWritten += WriteNumber32(channelStatusMode.size, pWriter);
		for(UInt32 i = 0; i < channelStatusMode.num; i++)
			nWritten += WriteNumber8(channelStatusMode.pElements[i], pWriter);
	}

	if(bFixedChannelStatusData)
	{
		nWritten += WriteNumber16(0x3d11, pWriter);
		nWritten += WriteNumber16(8+fixedChannelStatusData.num*fixedChannelStatusData.size, pWriter);
		nWritten += WriteNumber32(fixedChannelStatusData.num, pWriter);
		nWritten += WriteNumber32(fixedChannelStatusData.size, pWriter);
		for(UInt32 i = 0; i < fixedChannelStatusData.num; i++)
			nWritten += WriteBuffer(fixedChannelStatusData.ppItem[i], fixedChannelStatusData.size, pWriter);
	}

	if(bUserDataMode)
	{
		nWritten += WriteNumber16(0x3d12, pWriter);
		nWritten += WriteNumber16(8+userDataMode.num*userDataMode.size, pWriter);
		nWritten += WriteNumber32(userDataMode.num, pWriter);
		nWritten += WriteNumber32(userDataMode.size, pWriter);
		for(UInt32 i = 0; i < userDataMode.num; i++)
			nWritten += WriteNumber8(userDataMode.pElements[i], pWriter);
	}

	if(bFixedUserData)
	{
		nWritten += WriteNumber16(0x3d13, pWriter);
		nWritten += WriteNumber16(8+fixedUserData.num*fixedUserData.size, pWriter);
		nWritten += WriteNumber32(fixedUserData.num, pWriter);
		nWritten += WriteNumber32(fixedUserData.size, pWriter);
		for(UInt32 i = 0; i < fixedUserData.num; i++)
			nWritten += WriteBuffer(fixedUserData.ppItem[i], fixedUserData.size, pWriter);
	}

	return nWritten;
}



unsigned int RandomIndexPack::CalcLength()
{
	length = 12*numElements+4;
	return length;
}


unsigned int RandomIndexPack::Write(IWriter* pWriter)
{
unsigned int i;
DWORD nWritten = 0;

	CalcLength();
	nWritten=WriteBuffer(key, 16, pWriter);
	nWritten+=WriteBERLength(length, pWriter);
	

	for(i = 0; i < numElements; i++)
	{
		nWritten += WriteNumber32(pElements[i].bodySID, pWriter);
		nWritten += WriteNumber64(pElements[i].byteOffset, pWriter);
	}

	nWritten += WriteNumber32(overallLength, pWriter);

	return nWritten;
}


BOOL RandomIndexPack::Read(HANDLE hMXF, DWORD chunkSize)
{
unsigned int i;

	numElements = (chunkSize - 4) / 12;

	Alloc(numElements);
	for(i = 0; i < numElements; i++)
	{
		pElements[i].bodySID = GetNumber32(hMXF);
		pElements[i].byteOffset = GetNumber64(hMXF);
	}

	overallLength = GetNumber32(hMXF);

	return TRUE;
}


BOOL DMSegment::Read(HANDLE hMXF, DWORD chunkSize)
{
unsigned int localTag;
unsigned int size;
DWORD nRead;

	while(chunkSize > 0)
	{
		localTag = GetNumber16(hMXF);
		size = GetNumber16(hMXF);
		chunkSize -= 4;
		chunkSize -= size;

		switch(localTag)
		{
			case 0x3c0a:
				ReadFile(hMXF, instanceUID, 16, &nRead, NULL);
			break;
			case 0x0102:
				bGenerationUID = true;
				ReadFile(hMXF, generationUID, 16, &nRead, NULL);
			break;
			case 0x0201:
				ReadFile(hMXF, dataDefinition, 16, &nRead, NULL);
			break;
			case 0x0601:
				bEventStartPosition = true;
				eventStartPosition = GetNumber64(hMXF);
			break;
			case 0x0202:
				bDuration = true;
				duration = GetNumber64(hMXF);
			break;
			case 0x0602:
				bEventComment = true;
				eventComment = new wchar_t[(size/sizeof(wchar_t))+1];				
				ReadFile(hMXF, eventComment, size, &nRead, NULL);
				eventComment[(size/sizeof(wchar_t))] = 0x00;
				InvertUTF16String(eventComment);
			break;
			case 0x6102:
				bTrackID = true;
				trackID.Read(hMXF);
			break;
			case 0x6101:
				bDMFramework = true;
				ReadFile(hMXF, DMFramework, 16, &nRead, NULL);
			break;

			default:
				SetFilePointer(hMXF, size, NULL, FILE_CURRENT);
		}
	}
	return TRUE;
}

unsigned int DMSegment::CalcLength()
{
	length = 4+16 + // InstanceID
			 (bGenerationUID?(4+16):0) + // GenerationID
			 4+16 + // Data Definition
			 (bEventStartPosition?(4+8):0) + // Event Start Position
			 (bDuration?(4+8):0) + // Duration
			 (bEventComment?(4+((unsigned int)wcslen(eventComment))*2):0) +		// Event Comment
			 (bTrackID?(4+8+trackID.size*trackID.num):0) + // Track ID
			 (bDMFramework?(4+16):0); // DM Framework

	return length;
}

unsigned int DMSegment::Write(IWriter* pWriter)
{
DWORD nWritten;
unsigned int i;

	CalcLength();

	nWritten=WriteBuffer(key, 16, pWriter);
	nWritten+=WriteBERLength(length, pWriter);

	// Instance ID
	nWritten += WriteNumber16(0x3c0a, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteBuffer(instanceUID, 16, pWriter);

	// Generation UID
	if(bGenerationUID)
	{
		nWritten += WriteNumber16(0x0102, pWriter);
		nWritten += WriteNumber16(16, pWriter);
		nWritten += WriteBuffer(generationUID, 16, pWriter);
	}

	// Data Definition
	nWritten += WriteNumber16(0x0201, pWriter);
	nWritten += WriteNumber16(16, pWriter);
	nWritten += WriteBuffer(dataDefinition, 16, pWriter);

	// Event Start Position
	if(bEventStartPosition)
	{
		nWritten += WriteNumber16(0x0601, pWriter);
		nWritten += WriteNumber16(8, pWriter);
		nWritten += WriteNumber64(eventStartPosition, pWriter);
	}

	// Duration
	if(bDuration)
	{
		nWritten += WriteNumber16(0x0202, pWriter);
		nWritten += WriteNumber16(8, pWriter);
		nWritten += WriteNumber64(duration, pWriter);
	}

	// Event Comment
	if(bEventComment)
	{
		nWritten += WriteNumber16(0x0602, pWriter);
		nWritten += WriteNumber16((unsigned int)wcslen(eventComment)*2, pWriter);
		nWritten += WriteBuffer(eventComment, (unsigned int)wcslen(eventComment)*2, pWriter);
	}

	// Track IDs
	if(bTrackID)
	{
		nWritten += WriteNumber16(0x6102, pWriter);
		nWritten += WriteNumber16(8+trackID.num*trackID.size, pWriter);
		nWritten += WriteNumber32(trackID.num, pWriter);
		nWritten += WriteNumber32(trackID.size, pWriter);
		for(i = 0; i < trackID.num; i++)
			nWritten += WriteBuffer(trackID.ppItem[i], trackID.size, pWriter);
	}

	// DM Framework
	if(bDMFramework)
	{
		nWritten += WriteNumber16(0x6101, pWriter);
		nWritten += WriteNumber16(16, pWriter);
		nWritten += WriteBuffer(DMFramework, 16, pWriter);
	}

	return nWritten;
}