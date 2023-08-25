#ifndef __MXF_TYPES_H__
#define __MXF_TYPES_H__

#include "Interfaces.h"

// Keys

const unsigned char op1a_ul[16] =				{ 0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01, 0x00 };
const unsigned char dms_ul[14] =				{ 0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x04, 0x01, 0x01, 0x02 };

const unsigned char fillKey[16] =				{ 0x06, 0x0e, 0x2b, 0x34, 0x01, 0x01, 0x01, 0x01, 0x03, 0x01, 0x02, 0x10, 0x01, 0x00, 0x00, 0x00 };
const unsigned char klvFillDataKey[16] =		{ 0x06, 0x0e, 0x2b, 0x34, 0x01, 0x01, 0x01, 0x02, 0x03, 0x01, 0x02, 0x10, 0x01, 0x00, 0x00, 0x00 };
const unsigned char xmlDocumentTextKey[16] =	{ 0x06, 0x0e, 0x2b, 0x34, 0x01, 0x01, 0x01, 0x05, 0x03, 0x01, 0x02, 0x20, 0x01, 0x00, 0x00, 0x00 };

const unsigned char systemMetadataPackKey[16] = { 0x06, 0x0e, 0x2b, 0x34, 0x02, 0x05, 0x01, 0x01, 0x0d, 0x01, 0x03, 0x01, 0x04, 0x01, 0x01, 0x00 };
const unsigned char randomIndexPackKey[16] =	{ 0x06, 0x0e, 0x2b, 0x34, 0x02, 0x05, 0x01, 0x01, 0x0d, 0x01, 0x02, 0x01, 0x01, 0x11, 0x01, 0x00 };
const unsigned char headerKey[14] =				{ 0x06, 0x0e, 0x2b, 0x34, 0x02, 0x05, 0x01, 0x01, 0x0d, 0x01, 0x02, 0x01, 0x01, 0x02 };
const unsigned char footerKey[14] =				{ 0x06, 0x0e, 0x2b, 0x34, 0x02, 0x05, 0x01, 0x01, 0x0d, 0x01, 0x02, 0x01, 0x01, 0x04 };
const unsigned char primerPackKey[16] =			{ 0x06, 0x0e, 0x2b, 0x34, 0x02, 0x05, 0x01, 0x01, 0x0d, 0x01, 0x02, 0x01, 0x01, 0x05, 0x01, 0x00 };
const unsigned char partitionPackKey[13] =		{ 0x06, 0x0e, 0x2b, 0x34, 0x02, 0x05, 0x01, 0x01, 0x0d, 0x01, 0x02, 0x01, 0x01 };
const unsigned char bodyKey[14] =				{ 0x06, 0x0e, 0x2b, 0x34, 0x02, 0x05, 0x01, 0x01, 0x0d, 0x01, 0x02, 0x01, 0x01, 0x03 };
const unsigned char packageMetadataSetKey[15] = { 0x06, 0x0e, 0x2b, 0x34, 0x02, 0x43, 0x01, 0x01, 0x0d, 0x01, 0x03, 0x01, 0x04, 0x01, 0x02 };
const unsigned char indexTableKey[16] =			{ 0x06, 0x0e, 0x2b, 0x34, 0x02, 0x53, 0x01, 0x01, 0x0d, 0x01, 0x02, 0x01, 0x01, 0x10, 0x01, 0x00 };
const unsigned char metadataKey[13] =			{ 0x06, 0x0e, 0x2b, 0x34, 0x02, 0x53, 0x01, 0x01, 0x0d, 0x01, 0x01, 0x01, 0x01 };
const unsigned char darkdataKey[13] =			{ 0x06, 0x0e, 0x2b, 0x34, 0x02, 0x53, 0x01, 0x01, 0x0d, 0x01, 0x01, 0x01, 0x02 };

// DMS-1 Item
const unsigned char dms1ItemKey[13] =			{ 0x06, 0x0e, 0x2b, 0x34, 0x02, 0x53, 0x01, 0x01, 0x0d, 0x01, 0x04, 0x01, 0x01 };

//Generic Container Item
const unsigned char genericcontainerKey[16] = { 0x06, 0x0e, 0x2b, 0x34, 0x01, 0x02, 0x01, 0x01, 0x0d, 0x01, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00 };

//Generic Container Picture Item
const unsigned char gcpictureItemKey[13] =	{ 0x06, 0x0e, 0x2b, 0x34, 0x01, 0x02, 0x01, 0x01, 0x0d, 0x01, 0x03, 0x01, 0x15 };
// MPEG2 - Generic Container MPEG Picture Item - Frame Wrapped Picture Element
const unsigned char mpeg2VideoKey[16] =		{ 0x06, 0x0e, 0x2b, 0x34, 0x01, 0x02, 0x01, 0x01, 0x0d, 0x01, 0x03, 0x01, 0x15, 0x01, 0x05, 0x00 };

//Content Package Picture Item
const unsigned char cppictureItemKey[13] =	{ 0x06, 0x0e, 0x2b, 0x34, 0x01, 0x02, 0x01, 0x01, 0x0d, 0x01, 0x03, 0x01, 0x05 };
//IMX - Content Package D-10 Picture Item - MPEG2 422P@ML Element
const unsigned char imxVideoKey[16] =		{ 0x06, 0x0e, 0x2b, 0x34, 0x01, 0x02, 0x01, 0x01, 0x0d, 0x01, 0x03, 0x01, 0x05, 0x01, 0x01, 0x00 }; 

//Generic Container Compound Item
const unsigned char gccompoundItemKey[13] =	{ 0x06, 0x0E, 0x2B, 0x34, 0x01, 0x02, 0x01, 0x01, 0x0D, 0x01, 0x03, 0x01, 0x18 };
//Generic Container DV-DIF Compound Item
const unsigned char dvVideoKey[16] =		{ 0x06, 0x0E, 0x2B, 0x34, 0x01, 0x02, 0x01, 0x01, 0x0D, 0x01, 0x03, 0x01, 0x18, 0x00, 0x01, 0x00 }; 

//Generic Container Sound Item
const unsigned char gcsoundItemKey[13] =	{ 0x06, 0x0e, 0x2b, 0x34, 0x01, 0x02, 0x01, 0x01, 0x0d, 0x01, 0x03, 0x01, 0x16 };
//PCM S16LE - Generic Container Sound Item - Wave Frame-Wrapped Element
const unsigned char pcmAudioKey[16] =		{ 0x06, 0x0e, 0x2b, 0x34, 0x01, 0x02, 0x01, 0x01, 0x0d, 0x01, 0x03, 0x01, 0x16, 0x01, 0x01, 0x00 };
//Generic Container Sound Item - AES Frame-Wrapped Element
const unsigned char aesSoundKey[16] =		{ 0x06, 0x0e, 0x2b, 0x34, 0x01, 0x02, 0x01, 0x01, 0x0d, 0x01, 0x03, 0x01, 0x16, 0x01, 0x03, 0x00 };
//Generic Container MPEG Sound Item - Frame Wrapped Sound Element
const unsigned char mpegSoundKey[16] =		{ 0x06, 0x0e, 0x2b, 0x34, 0x01, 0x02, 0x01, 0x01, 0x0d, 0x01, 0x03, 0x01, 0x16, 0x01, 0x05, 0x00 };

//Content Package Sound Item
const unsigned char cpsoundItemKey[13] =	{ 0x06, 0x0e, 0x2b, 0x34, 0x01, 0x02, 0x01, 0x01, 0x0d, 0x01, 0x03, 0x01, 0x06 };
//Content Package 8-channel AES3 Sound Item
const unsigned char aes3Ch8AudioKey[16] =	{ 0x06, 0x0e, 0x2b, 0x34, 0x01, 0x02, 0x01, 0x01, 0x0d, 0x01, 0x03, 0x01, 0x06, 0x01, 0x10, 0x00 };

// Generic Container Data Item
const unsigned char gcdataItemKey[13] =		{ 0x06, 0x0e, 0x2b, 0x34, 0x01, 0x02, 0x01, 0x01, 0x0d, 0x01, 0x03, 0x01, 0x17 };

// Content Package Data Item
const unsigned char cpdataItemKey[13] =		{ 0x06, 0x0e, 0x2b, 0x34, 0x01, 0x02, 0x01, 0x01, 0x0d, 0x01, 0x03, 0x01, 0x07 };

typedef unsigned __int64 UInt64;
typedef unsigned int	 UInt32;
typedef unsigned short	 UInt16;
typedef unsigned char	 UInt8;

typedef __int64 Int64;
typedef int		Int32;
typedef short	Int16;
typedef char	Int8;

//typedef UInt64 Timestamp;
union Timestamp
{
	UInt64 i64Data;
	struct
	{
		UInt8 quartermsec;
		UInt8 second;
		UInt8 minute;
		UInt8 hour;
		UInt8 day;
		UInt8 month;
		UInt16 year;
	} st;
};
typedef UInt64 Position;
typedef UInt64 Length;
typedef UInt8 Enum;

typedef unsigned char UL[16];
typedef unsigned char UUID_[16];
typedef unsigned char UMID[32];

typedef UInt16 ProductVersion[5];

typedef wchar_t* String;


struct Rational
{
	UInt32 num;
	UInt32 den;

	Rational()
	{
		num = 0;
		den = 1;
	}

	Rational(UInt32 num, UInt32 den)
	{
		this->num = num;
		this->den = den;
	}

	Rational& operator=(Rational& other)
	{
		this->num = other.num;
		this->den = other.den;
		return *this;
	}
};

struct DateTimeStamp
{
	UInt8 type;
	UInt8 data[16];
};


struct Batch
{
	UInt32 num;
	UInt32 size;

	unsigned char** ppItem;

	Batch()
	{
		num = 0;
		size = 0;
		ppItem = NULL;
	}

	void Alloc(UInt32 num, UInt32 size)
	{
		unsigned int i;
		this->num = num;
		this->size = size;
		ppItem = new unsigned char*[num];
		for(i = 0; i < num; i++)
			ppItem[i] = new unsigned char[size];			
	}

	~Batch()
	{
		if(ppItem)
		{
			for(unsigned int i=0; i < num; i++)
				delete[] ppItem[i];
			delete[] ppItem;
		}
	}

	BOOL Read(HANDLE hMXF);
};


// SMPTE 377
// Seção 6.1 (Table 2)
struct PartitionPack
{
	UL key;
	UInt32 length;
	UInt16 majorVersion;
	UInt16 minorVersion;
	UInt32 kagSize;
	UInt64 thisPartition;
	UInt64 previousPartition;
	UInt64 footerPartition;
	UInt64 headerByteCount;
	UInt64 indexByteCount;
	UInt32 indexSID;
	UInt64 bodyOffset;
	UInt32 bodySID;
	UL operationPattern;
	Batch essenceContainers;

	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter);

	BOOL Read(HANDLE hMXF, DWORD chunkSize);
};

struct DeltaEntryArray
{
	unsigned int NDE;
	unsigned int length;
	struct DeltaEntry
	{
		char posTableIndex;
		unsigned char slice;
		unsigned int elementDelta;
	};
	DeltaEntry* pDeltaEntry;

	void Alloc(int num)
	{
		if(pDeltaEntry) delete[] pDeltaEntry;
		length = 6;
		NDE = num;
		pDeltaEntry = new DeltaEntry[num];
	}

	DeltaEntryArray()
	{
		pDeltaEntry = NULL;
	}
	~DeltaEntryArray()
	{
		if(pDeltaEntry)
			delete[] pDeltaEntry;
	}


};


struct IndexEntryArray
{
	unsigned int NIE;
	unsigned int length;
	unsigned int NPE;
	unsigned int NSL;
	struct IndexEntry
	{
		char temporalOffset;
		char keyFrameOffset;
		unsigned char flags;
		unsigned __int64 streamOffset;
		unsigned int* pSliceOffset;
		Rational* pPosTable;

		IndexEntry()
		{
			pSliceOffset = NULL;
			pPosTable = NULL;
		}

		~IndexEntry()
		{
			if(pSliceOffset) delete[] pSliceOffset;
			if(pPosTable) delete[] pPosTable;
		}
	};
	IndexEntry* pIndexEntry;

	void Alloc(int num, int numSlices, int numPosTable)
	{
		if(pIndexEntry) delete[] pIndexEntry;
		NIE = num;
		NSL = numSlices;
		NPE = numPosTable;
		length = 11 + (NSL*4) + (NPE*8);
		pIndexEntry = new IndexEntry[num];
		for(int i=0; NSL && (i < num); i++)
			pIndexEntry[i].pSliceOffset = new unsigned int[NSL];
		for(int i=0; NPE && (i < num); i++)
			pIndexEntry[i].pPosTable = new Rational[NSL];
	}


	IndexEntryArray()
	{
		pIndexEntry = NULL;
		NIE = NSL = NPE = 0;
	}

	~IndexEntryArray()
	{
		if(pIndexEntry) delete[] pIndexEntry;
	}
};


// SMPTE 377
// Seção 10.2.3 (Table 19)
struct IndexTableSegment
{
	UL key;
	UInt32 length;	
	UUID_ instanceID;
	Rational indexEditRate;
	UInt64 indexStartPosition;
	UInt64 indexDuration;
	bool bEditUnitByteCount;
	UInt32 editUnitByteCount;
	bool bIndexSID;
	UInt32 indexSID;
	UInt32 bodySID;
	bool bSliceCount;
	UInt8 sliceCount;
	bool bPosTableCount;
	UInt8 posTableCount;
	bool bDeltaEntryArray;
	DeltaEntryArray deltaEntryArray;
	bool bIndexEntryArray;
	IndexEntryArray indexEntryArray;

	IndexTableSegment()
	{
		 bEditUnitByteCount = false;
		 bIndexSID = false;
		 bSliceCount = false;
		 bPosTableCount = false;
		 bDeltaEntryArray = false;
		 bIndexEntryArray = false;
	}

	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter);
	BOOL Read(HANDLE hMXF, DWORD size);
};


// SMPTE 377
// Seção A.1
struct Preface
{
	UL key;
	unsigned int length;
	UUID_ instanceUID;
	bool bGenerationUID;
	UUID_ generationUID;
	Timestamp lastModifiedDate;
	UInt16 version;
	bool bObjectModelVersion;
	UInt32 objectModelVersion;
	bool bPrimaryPackage;
	UUID_ primaryPackage;
	Batch identifications;
	UL contentStorage;
	UL operationalPattern;
	Batch essenceContainers;
	Batch DMSchemes;

	Preface()
	{
		bGenerationUID = false;
		bObjectModelVersion = false;
		bPrimaryPackage = false;
	}

	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter);
	BOOL Read(HANDLE hMXF, DWORD chunkSize);
};


// SMPTE 377
// Seção A.2
struct Identification
{
	UL key;
	unsigned int length;
	UUID_ instanceUID;
	UUID_ thisGenerationUID;
	String companyName;
	String productName;
	bool bProductVersion;
	ProductVersion productVersion;
	String versionString;
	UUID_ productUID;
	Timestamp modificationDate;
	bool bToolkitVersion;
	ProductVersion toolkitVersion;
	bool bPlatform;
	String platform;

	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter);
	BOOL Read(HANDLE hMXF, long chunkSize);

	Identification()
	{
		bProductVersion = false;
		bToolkitVersion = false;
		bPlatform = false;
		companyName = NULL;
		productName = NULL;
		versionString = NULL;
		platform = NULL;
	}

	~Identification()
	{
		if(companyName) delete[] companyName;
		if(productName) delete[] productName;
		if(versionString) delete[] versionString;
		if(platform) delete[] platform;
	}
};


// SMPTE 377
// Seção A.3
struct ContentStorage
{
	UL key;
	unsigned int length;
	UUID_ instanceUID;
	bool bGenerationUID;
	UUID_ generationUID;
	Batch packages;
	bool bEssenceContainerData;
	Batch essenceContainerData;

	ContentStorage()
	{
		bGenerationUID = false;
		bEssenceContainerData = false;
	}

	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter);
	BOOL Read(HANDLE hMXF, long chunkSize);
};


// SMPTE 377
// Seção A.4
struct EssenceContainerData
{
	UL key;
	unsigned int length;
	UUID_ instanceUID;
	UMID linkedPackageUID;
	bool bGenerationUID;
	UUID_ generationUID;
	bool bIndexSID;
	UInt32 indexSID;
	UInt32 bodySID;

	EssenceContainerData()
	{
		bGenerationUID = false;
		bIndexSID = false;
	}

	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter);
	BOOL Read(HANDLE hMXF, DWORD chunkSize);
};


// SMPTE 377
// Seção B.1
struct GenericPackage
{
	UL key;
	unsigned int length;
	UUID_ instanceUID;
	UMID  packageUID;
	bool bGenerationUID;
	UUID_ generationUID;
	bool bName;
	String name;
	Timestamp packageCreationDate;
	Timestamp packageModifiedDate;
	Batch tracks;
	UL descriptor;

	GenericPackage()
	{
		bGenerationUID = false;
		bName = false;
		name = NULL;
	}

	~GenericPackage()
	{
		if(name) delete[] name;
	}

	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter, BOOL bWriteKL = TRUE);
	BOOL Read(HANDLE hMXF, long chunkSize);
};


// SMPTE 377
// Seções B.5, B.8, B.11, B.14
struct Track
{
	UL key;
	unsigned int length;
	UUID_ instanceUID;
	bool bGenerationUID;
	UUID_ generationUID;
	bool bTrackID;
	UInt32 trackID;
	UInt32 trackNumber;
	bool bTrackName;
	String trackName;
	Rational editRate;
	Position origin;
	UL sequence;

	Track()
	{
		bGenerationUID = false;
		bTrackID = false;
		bTrackName = false;
		trackName = NULL;
	}

	~Track()
	{
		if(trackName) delete[] trackName;
	}

	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter);
	BOOL Read(HANDLE hMXF, long chunkSize);
};

// SMPTE 377
// Seção B.17.3
struct StaticTrack
{
	UL key;
	unsigned int length;
	UUID_ instanceUID;
	bool bGenerationUID;
	UUID_ generationUID;
	bool bTrackID;
	UInt32 trackID;
	UInt32 trackNumber;
	bool bTrackName;
	String trackName;
	UL sequence;

	StaticTrack()
	{
		 bGenerationUID = false;
		 bTrackID = false;
		 bTrackName = false;
		 trackName = NULL;
	}

	~StaticTrack()
	{
		if(trackName)delete[] trackName;
	}

	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter);
	BOOL Read(HANDLE hMXF, DWORD chunkSize);
};


// SMPTE 377
// Seções B.10, B.13, B.16
struct SourceClip
{
	unsigned char key[16];
	unsigned int length;
	unsigned char instanceUID[16];
	bool bGenerationUID;
	unsigned char generationUID[16]; // optional
	unsigned char dataDefinition[16];
	unsigned __int64 startPosition;
	unsigned __int64 duration;
	bool bSourcePackageID;
	unsigned char sourcePackageID[32];
	unsigned int sourceTrackID;

	SourceClip()
	{
		bGenerationUID = false;
		bSourcePackageID = false;
	}

	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter);
	BOOL Read(HANDLE hMXF, DWORD chunkSize);
};


// SMPTE 377
// Seções B.6, B.9, B.12, B.15
struct Sequence
{
	unsigned char key[16];
	unsigned int length;
	unsigned char instanceUID[16];
	bool bGenerationUID;
	unsigned char generationUID[16];
	unsigned char dataDefinition[16];
	unsigned __int64 duration;
	Batch structuralComponents;
	Sequence()
	{
		bGenerationUID = false;
	}

	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter);
	BOOL Read(HANDLE hMXF, long chunkSize);
};


// SMPTE 377
// Seção B.7
struct TimecodeComponent
{
	unsigned char key[16];
	unsigned int length;
	unsigned char instanceUID[16];
	bool bGenerationUID;
	unsigned char generationUID[16];
	unsigned char dataDefinition[16];
	unsigned __int64 duration;
	unsigned short roundedTimecodeBase;
	unsigned __int64 startTimecode;
	unsigned char dropFrame;

	TimecodeComponent()
	{
		bGenerationUID = false;
	}

	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter);
	BOOL Read(HANDLE hMXF, long chunkSize);
};


// SMPTE 377
// Seção C.2
struct SourcePackage
{
	UL key;
	UInt32 length;
	GenericPackage package;
	
	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter);
	BOOL Read(HANDLE hMXF, long chunkSize);
};


// SMPTE 377
// Seção D.1
struct FileDescriptor
{
	UL key;
	unsigned int length;
	UUID_ instanceUID;
	bool bGenerationUID;
	UUID_ generationUID;
	bool bLinkedTrackID;
	UInt32 linkedTrackID;
	Rational sampleRate;
	bool bContainerDuration;
	Length containerDuration;
	UL essenceContainer;
	bool bCodec;
	UL codec;
	bool bLocators;
	Batch locators;

	FileDescriptor()
	{
		bGenerationUID = false;
		bLinkedTrackID = false;
		bContainerDuration = false;
		bCodec = false;
		bLocators = false;
	}

	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter, BOOL bWriteKL);
	BOOL Read(HANDLE hMXF, long chunkSize);
};


// SMPTE 377
// Seção D.5
struct MultipleDescriptor
{
	UL key;
	UInt32 length;
	FileDescriptor fileDescriptor;
	Batch subDescriptorUIDs;

	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter);
	BOOL Read(HANDLE hMXF, DWORD chunkSize);
};


// SMPTE 377
// Seção D.2.1
struct GenericPictureEssenceDescriptor
{
	UL key;
	UInt32 length;
	FileDescriptor fileDescriptor;
	bool bSignalStandard;
	Enum signalStandard; // SMPTE 377, E.2.3
	UInt8 frameLayout;  // E.2.2
	UInt32 storedWidth; // E.2.9
	UInt32 storedHeight; // E.2.10
	bool bStoredF2Offset; 
	Int32 storedF2Offset; // E.2.22
	bool bSampledWidth; 
	UInt32 sampledWidth;  // E.2.11
	bool bSampledHeight;
	UInt32 sampledHeight; // E.2.12
	bool bSampledXOffset;
	Int32 sampledXOffset; // E.2.13
	bool bSampledYOffset;
	Int32 sampledYOffset; // E.2.14
	bool bDisplayHeight;
	UInt32 displayHeight;  // E.2.18
	bool bDisplayWidth;
	UInt32 displayWidth; // E.2.19
	bool bDisplayXOffset;
	Int32 displayXOffset; // E.2.19
	bool bDisplayYOffset;
	Int32 displayYOffset;  // E.2.20
	bool bDisplayF2Offset;
	Int32 displayF2Offset; // E.2.21
	Rational aspectRatio; // E.2.7
	bool bActiveFormatDescriptor;
	UInt8 activeFormatDescriptor; // E.2.8
	UInt32 videoLineMap[2];  // E.2.15
	bool bAlphaTransparency;
	UInt8 alphaTransparency; // E.2.26
	bool bCaptureGamma;
	UL captureGamma; // E.2.27
	bool bImageAlignmentOffset;
	UInt32 imageAlignmentOffset; // E.2.28
	bool bImageStartOffset;
	UInt32 imageStartOffset; // E.2.29
	bool bImageEndOffset;
	UInt32 imageEndOffset; // E.2.30
	bool bFieldDominance;
	UInt8 fieldDominance; // E.2.25
	bool bPictureEssenceCoding;
	UL pictureEssenceCoding; // E.2.25

	GenericPictureEssenceDescriptor()
	{
		 bSignalStandard= false;
		 bStoredF2Offset= false;
		 bSampledWidth= false;
		 bSampledHeight= false;
		 bSampledXOffset= false;
		 bSampledYOffset= false;
		 bDisplayWidth= false;
		 bDisplayHeight= false;
		 bDisplayXOffset= false;
		 bDisplayYOffset= false;
		 bDisplayF2Offset= false;
		 bActiveFormatDescriptor= false;
		 bAlphaTransparency= false;
		 bCaptureGamma= false;
		 bImageAlignmentOffset= false;
		 bImageStartOffset= false;
		 bImageEndOffset= false;
		 bFieldDominance= false;
		 bPictureEssenceCoding= false;
	}

	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter, BOOL bWriteKL);
	BOOL Read(HANDLE hMXF, DWORD chunkSize);
};


// SMPTE 377
// Seção D.2.2
struct CDCIPictureEssenceDescriptor
{
	UL key;
	UInt32 length;
	GenericPictureEssenceDescriptor genericPictureEssenceDescriptor;
	UInt32 componentDepth;
	UInt32 horizontalSubsampling;
	bool bVerticalSubsampling;
	UInt32 verticalSubsampling;
	bool bColorSiting;
	UInt8 colorSiting;
	bool bReversedByteOrder;
	UInt8 reversedByteOrder;
	bool bPaddingBits;
	Int16 paddingBits;
	bool bAlphaSampleDepth;
	UInt32 alphaSampleDepth;
	bool bBlackRefLevel;
	UInt32 blackRefLevel;
	bool bWhiteRefLevel;
	UInt32 whiteRefLevel;
	bool bColorRange;
	UInt32 colorRange;

	CDCIPictureEssenceDescriptor()
	{
		 bVerticalSubsampling = false;
		 bColorSiting = false;
		 bReversedByteOrder = false;
		 bPaddingBits = false;
		 bAlphaSampleDepth = false;
		 bBlackRefLevel = false;
		 bWhiteRefLevel = false;
		 bColorRange = false;
	}

	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter, BOOL bWriteKL);
	BOOL Read(HANDLE hMXF, DWORD chunkSize);
};


// SMPTE 377
// Seção D.3
struct GenericSoundEssenceDescriptor
{
	UL key;
	UInt32 length;
	FileDescriptor fileDescriptor; // D.1
	Rational audioSamplingRate;
	bool bLockedUnlocked;
	UInt8 lockedUnlocked;
	bool bAudioRefLevel;
	Int8 audioRefLevel;
	bool bElectroSpatialFormulation;
	UInt8 electroSpatialFormulation;
	UInt32 channelCount;
	UInt32 quantizationBits;
	bool bDialNorm;
	Int8 dialNorm;
	bool bSoundEssenceCompression;
	UL soundEssenceCompression;

	GenericSoundEssenceDescriptor()
	{
		bLockedUnlocked = false;
		bAudioRefLevel = false;
		bElectroSpatialFormulation = false;
		bDialNorm = false;
		bSoundEssenceCompression = false;
	}

	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter, BOOL bWriteKL);
	BOOL Read(HANDLE hMXF, DWORD chunkSize);
};


// SMPTE 385
// Seção 5.2.1
struct SystemMetadataPack
{
	UL key;
	UInt32 length;
	UInt8 systemMetadataBitmap;
	UInt8 contentPackageRate;
	UInt8 contentPackageType;
	UInt16 channelHandle;
	UInt16 continuityCount;
	UL universalLabel;
	DateTimeStamp creationDateTimeStamp;
	DateTimeStamp userDateTimeStamp;

	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter);
	BOOL Read(HANDLE hMXF);
};

struct SMPTE12MTimecode
{
	unsigned int frameUnits:4;
	unsigned int frameTens:2;
	unsigned int dfFlag:1;
	unsigned int cfFlag:1;

	unsigned int secondsUnits:4;
	unsigned int secondsTens:3;
	unsigned int fp:1;

	unsigned int minutesUnits:4;
	unsigned int minutesTens:3;
	unsigned int b0:1;

	unsigned int hoursUnits:4;
	unsigned int hoursTens:2;
	unsigned int b1:1;
	unsigned int b2:1;

	unsigned int bg1:4;
	unsigned int bg2:4;

	unsigned int bg3:4;
	unsigned int bg4:4;

	unsigned int bg5:4;
	unsigned int bg6:4;

	unsigned int bg7:4;
	unsigned int bg8:4;
};

struct LocalTagEntry
{
	UInt16 localTag;
	UL uid;
};

// SMPTE 377
// Seção 8.2 (Table 12)
struct LocalTagEntryBatch
{
	UInt32 numberOfItems;
	UInt32 itemLength;
	LocalTagEntry* pItems;

	LocalTagEntryBatch()
	{
		numberOfItems = 0;
		pItems = NULL;
	}

	~LocalTagEntryBatch()
	{
		if(pItems)
			delete[] pItems;
	}
};

// SMPTE 377
// Seção 8.2 (Table 11)
struct PrimerPack
{
	UL key;
	UInt32 length;
	LocalTagEntryBatch localTagEntryBatch;

	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter);
	BOOL Read(HANDLE hMXF);

	UInt16 FindLocalTag(UInt64 itemDesignator);
	UInt64 FindItemDesignator(UInt16 localTag);
};


// SMPTE 381
//
struct MPEG2VideoDescriptor
{
	UL key;
	UInt32 length;
	CDCIPictureEssenceDescriptor cdciPictureEssenceDescriptor;
	bool bSingleSequence;
	UInt8 singleSequence;
	bool bConstantBFrames;
	UInt8 constantBFrames;
	bool bCodedContentType;
	UInt8 codedContentType;
	bool bLowDelay;
	UInt8 lowDelay;
	bool bClosedGOP;
	UInt8 closedGOP;
	bool bIdenticalGOP;
	UInt8 identicalGOP;
	bool bMaxGOP;
	UInt16 maxGOP;
	bool bBPictureCount;
	UInt16 bPictureCount;
	bool bBitRate;
	UInt32 bitRate;
	bool bProfileAndLevel;
	UInt8 profileAndLevel;

	MPEG2VideoDescriptor()
	{
		bSingleSequence = false;
		bConstantBFrames = false;
		bCodedContentType = false;
		bLowDelay = false;
		bClosedGOP = false;
		bIdenticalGOP = false;
		bMaxGOP = false;
		bBPictureCount = false;
		bBitRate = false;
		bProfileAndLevel = false;
	}

	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter, PrimerPack* pPrimerPack);
	BOOL Read(HANDLE hMXF, PrimerPack* pPrimerPack, DWORD chunkSize);
};


struct WaveAudioEssenceDescriptor
{
	UL key;
	UInt32 length;
	GenericSoundEssenceDescriptor genericSoundEssenceDescriptor;
	
	UInt16 blockAlign;

	bool bSequenceOffset;
	UInt8 sequenceOffset;
	UInt32 avgBps;
	bool bChannelAssignment;
	UL channelAssignment;
	bool bPeakEnvelopeVersion;
	UInt32 peakEnvelopeVersion;
	bool bPeakEnvelopeFormat;
	UInt32 peakEnvelopeFormat;
	bool bPointsPerPeakValue;
	UInt32 pointsPerPeakValue;
	bool bPeakEnvelopeBlockSize;
	UInt32 peakEnvelopeBlockSize;
	bool bPeakChannels;
	UInt32 peakChannels;
	bool bPeakFrames;
	UInt32 peakFrames;
	bool bPeakOfPeaksPosition;
	Position peakOfPeaksPosition;
	bool bPeakEnvelopeTimestamp;
	Timestamp peakEnvelopeTimestamp;
	bool bPeakEnvelopeData;
	unsigned char* peakEnvelopeData;

	WaveAudioEssenceDescriptor()
	{
		bSequenceOffset = false;
		bChannelAssignment = false;
		bPeakEnvelopeVersion = false;
		bPeakEnvelopeFormat = false;
		bPointsPerPeakValue = false;
		bPeakEnvelopeBlockSize = false;
		bPeakChannels = false;
		bPeakFrames = false;
		bPeakOfPeaksPosition = false;
		bPeakEnvelopeTimestamp = false;
		bPeakEnvelopeData = false;
		peakEnvelopeData = NULL;
	}

	BOOL Read(HANDLE hMXF, DWORD chunkSize);
	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter, BOOL bWriteKL);
};


struct UInt8Array
{
	UInt32 num;
	UInt32 size;
	UInt8* pElements;

	void Alloc(int n)
	{
		if(pElements) delete[] pElements;
		num = n;
		pElements = new UInt8[num];
	}

	UInt8Array()
	{
		num = 0;
		size = 1;
		pElements = NULL;
	}


	~UInt8Array()
	{
		if(pElements)
			delete[] pElements;
	}

	void Read(HANDLE hMXF);
};

struct AES3AudioEssenceDescriptor
{
	UL key;
	UInt32 length;

	WaveAudioEssenceDescriptor waveAudioEssenceDescriptor;

	bool bEmphasis;
	UInt8 emphasis;
	bool bBlockStartOffset;
	UInt16 blockStartOffset;
	bool bAuxBitsMode;
	UInt8 auxBitsMode;
	bool bChannelStatusMode;
	UInt8Array channelStatusMode;
	bool bFixedChannelStatusData;
	Batch fixedChannelStatusData;
	bool bUserDataMode;
	UInt8Array userDataMode;
	bool bFixedUserData;
	Batch fixedUserData;

	AES3AudioEssenceDescriptor()
	{
		 bEmphasis = false;
		 bBlockStartOffset = false;
		 bAuxBitsMode = false;
		 bChannelStatusMode = false;
		 bFixedChannelStatusData = false;
		 bUserDataMode = false;
		 bFixedUserData = false;
	}

	BOOL Read(HANDLE hMXF, DWORD chunkSize);
	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter);
};

struct RandomIndexPackElement
{
	UInt32 bodySID;
	UInt64 byteOffset;
};

struct RandomIndexPack
{
	UL key;
	UInt32 length;

	UInt32 numElements;
	RandomIndexPackElement* pElements;

	UInt32 overallLength;

	RandomIndexPack()
	{
		numElements = 0;
		pElements = NULL;
	}

	~RandomIndexPack()
	{
		if(pElements)
			delete[] pElements;
	}

	void Alloc(int num)
	{
		if(pElements) delete[] pElements;
		numElements = num;
		pElements = new RandomIndexPackElement[num];
	}

	BOOL Read(HANDLE hMXF, DWORD chunkSize);
	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter);
};


struct DMSegment
{
	UL key;
	UInt32 length;

	UUID_ instanceUID;
	bool bGenerationUID;
	UUID_ generationUID; // optional
	UL dataDefinition;
	bool bEventStartPosition;
	Position eventStartPosition;
	bool bDuration;
	unsigned __int64 duration;
	bool bEventComment;
	String eventComment;
	bool bTrackID;
	Batch trackID;
	bool bDMFramework;
	UL DMFramework;

	DMSegment()
	{
		 bGenerationUID = false;
		 bEventStartPosition = false;
		 bDuration = false;
		 bEventComment = false;
		 bTrackID = false;
		 bDMFramework = false;
		 eventComment = NULL;
	}

	~DMSegment()
	{
		if(eventComment) delete[] eventComment;
	}

	BOOL Read(HANDLE hMXF, DWORD chunkSize);
	unsigned int CalcLength();
	unsigned int Write(IWriter* pWriter);
};

#endif