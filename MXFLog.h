#pragma once

#include "MXFTypes.h"
#include "MXFUtils.h"


class MXFLogger
{
public:
	MXFLogger(const wchar_t* fnameMXF);
	~MXFLogger();

	BOOL Save(const wchar_t* fnameDump);

	static unsigned long threadProc(MXFLogger* pLogger);

private:
	HANDLE		m_threadHnd;
	CString		m_fnameMXF;
	CString		m_fnameDump;

	BOOL WriteMXF(const wchar_t* fnameMXF, const wchar_t* fnameDump);

	void WriteMXFPartitionPack(PartitionPack& ppack, FILE* fpDump);
	void WriteMXFPrimerPack(PrimerPack& pp, FILE* fpDump);
	void WriteMXFIndexTable(IndexTableSegment& its, FILE* fpDump);
	void WriteMXFPreface(Preface& p, FILE* fpDump);
	void WriteMXFIdentification(Identification& id, FILE* fpDump);
	void WriteMXFContentStorage(ContentStorage& cs, FILE* fpDump);
	void WriteMXFEssenceContainerData(EssenceContainerData& ecd, FILE* fpDump);
	void WriteMXFGenericPackage(GenericPackage& gp, FILE* fpDump);
	void WriteMXFTrack(Track& t, FILE* fpDump);
	void WriteMXFStaticTrack(StaticTrack& t, FILE* fpDump);
	void WriteMXFMetadataSourceClip(SourceClip& clip, FILE* fpDump);
	void WriteMXFMetadataSequence(Sequence& seq, FILE* fpDump);
	void WriteMXFMetadataTimecodeComponent(TimecodeComponent tc, FILE* fpDump);
	void WriteMXFSourcePackage(SourcePackage& sp, FILE* fpDump);
	void WriteMXFFileDescriptor(FileDescriptor& fd, FILE* fpDump);
	void WriteMXFMultipleDescriptor(MultipleDescriptor& md, FILE* fpDump);
	void WriteMXFGenericPictureEssenceDescriptor(GenericPictureEssenceDescriptor& gped, FILE* fpDump);
	void WriteMXFCDCIPictureEssenceDescriptor(CDCIPictureEssenceDescriptor& cped, FILE* fpDump);
	void WriteMXFGenericSoundEssenceDescriptor(GenericSoundEssenceDescriptor& gsed, FILE* fpDump);
	void WriteMXFSystemMetadataPack(SystemMetadataPack& smp, FILE* fpDump);
	void WriteMXFMPEG2VideoDescriptor(MPEG2VideoDescriptor& vd, FILE* fp);
	void WriteMXFWaveAudioEssenceDescriptor(WaveAudioEssenceDescriptor& wd, FILE* fp);
	void WriteMXFAES3AudioEssenceDescriptor(AES3AudioEssenceDescriptor& ad, FILE* fp);
	void WriteMXFRandomIndexPack(RandomIndexPack& rip, FILE* fp);
	void WriteMXFDMSegment(DMSegment& dms, FILE* fpDump);


};