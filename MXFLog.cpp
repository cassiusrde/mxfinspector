#include "stdafx.h"

#include "MXFLog.h"


MXFLogger::MXFLogger(const wchar_t* fnameMXF)
{
	m_fnameMXF = fnameMXF;
	m_threadHnd = 0;
}

MXFLogger::~MXFLogger()
{
	if( m_threadHnd != 0 )
	{
		WaitForSingleObject(m_threadHnd, INFINITE);
		CloseHandle(m_threadHnd);
		m_threadHnd = 0;	
	}
}

BOOL MXFLogger::Save(const wchar_t *fnameDump)
{
	m_fnameDump = fnameDump;

	if( m_threadHnd != 0 )
	{
		WaitForSingleObject(m_threadHnd, INFINITE);
		CloseHandle(m_threadHnd);
		m_threadHnd = 0;	
	}

	m_threadHnd = ::CreateThread(0, 0, LPTHREAD_START_ROUTINE(threadProc), this, 0, 0);

	return TRUE;
}

unsigned long MXFLogger::threadProc(MXFLogger* pLogger)
{
	return pLogger->WriteMXF(pLogger->m_fnameMXF.GetString(), pLogger->m_fnameDump.GetString()); 	
}


BOOL MXFLogger::WriteMXF(const wchar_t* fnameMXF, const wchar_t* fnameDump)
{
	int i;
	unsigned char keyValue[16];
	unsigned char ch;
	DWORD size, szSize, nRead;
	BOOL bSeek, bCont;
	HANDLE hMXF;
	FILE* fpDump;
	PrimerPack primerPack;

	hMXF = CreateFile(fnameMXF, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hMXF == INVALID_HANDLE_VALUE)
	{
		printf("Erro ao abrir arquivo MXF\n");
		return FALSE;
	}

	fpDump = _wfopen(fnameDump, L"wb");
	if(!fpDump)
	{
		printf("Erro ao criar arquivo Dump\n");
		CloseHandle(hMXF);
		return FALSE;
	}

	bCont = TRUE;
	unsigned __int64 filePos;
		
	while(bCont)
	{
		filePos = GetFilePosition(hMXF);
		ReadFile(hMXF, keyValue, 16, &nRead, NULL);
		if(nRead < 16)
			break; // EOF
		
		fprintf(fpDump, "<%016I64x> ", filePos);

		ReadFile(hMXF, &ch, 1, &nRead, NULL);
		szSize = ch&0x0f;
		size = (DWORD)GetNumber(hMXF, szSize);

		for(i = 0; i < 16; i++)
			fprintf(fpDump, "%02x ", keyValue[i]);
		
		fprintf(fpDump, ": %d bytes", size);

		bSeek = TRUE;	

		if(!memcmp(keyValue, gcpictureItemKey, sizeof(gcpictureItemKey)) )
		{
			switch(keyValue[14])
			{
				case 0x05: fprintf(fpDump, "[PICTURE]<MPEG2>"); break;
				default: fprintf(fpDump, "[PICTURE]<UNKNOWN>"); break;
			}

			bSeek = TRUE;
		}

		if(!memcmp(keyValue, cppictureItemKey, sizeof(cppictureItemKey)) )
		{
			switch(keyValue[14])
			{
				case 0x01: fprintf(fpDump, "[PICTURE]<IMAX>"); break;
				default: fprintf(fpDump, "[PICTURE]<UNKNOWN>"); break;
			}

			bSeek = TRUE;
		}
		
		if(!memcmp(keyValue, gcsoundItemKey, sizeof(gcsoundItemKey)) )
		{
			switch(keyValue[14])
			{
				case 0x01: fprintf(fpDump, "[SOUND]<PCM S16LE>"); break;
				case 0x03: fprintf(fpDump, "[SOUND]<AES>"); break;
				case 0x05: fprintf(fpDump, "[SOUND]<MPEG>"); break;
				default: fprintf(fpDump, "[SOUND]<UNKNOWN>"); break;					
			}

			bSeek = TRUE;
		}
		
		if(!memcmp(keyValue, cpsoundItemKey, sizeof(cpsoundItemKey)) )
		{
			switch(keyValue[14])
			{
				case 0x010: fprintf(fpDump, "[SOUND]<AES3 8CH>"); break;
				default: fprintf(fpDump, "[SOUND]<UNKNOWN>"); break;	
			}
			bSeek = TRUE;
		}		
		
		if(!memcmp(keyValue, headerKey, sizeof(headerKey)))
		{
			int status = keyValue[14];
			fprintf(fpDump, "[HEADER] => status = %x\r\n", status);

			PartitionPack pack;
			pack.Read(hMXF, size);
			WriteMXFPartitionPack(pack, fpDump);
			bSeek = FALSE;
		}

		if(!memcmp(keyValue, bodyKey, sizeof(bodyKey)))
		{
			int status = keyValue[14];
			fprintf(fpDump, "[BODY] => status = %x\r\n", status);

			PartitionPack pack;
			pack.Read(hMXF, size);
			WriteMXFPartitionPack(pack, fpDump);
			bSeek = FALSE;
		}

		if(!memcmp(keyValue, primerPackKey, sizeof(primerPackKey)))
		{
			fprintf(fpDump, "[PRIMER PACK]\r\n");

			primerPack.Read(hMXF);
			WriteMXFPrimerPack(primerPack, fpDump);
			bSeek = FALSE;
		}

		if(!memcmp(keyValue, footerKey, sizeof(footerKey)))
		{
			int status = keyValue[14];
			fprintf(fpDump, "[FOOTER] => status = %x\r\n", status);

			PartitionPack pack;
			pack.Read(hMXF, size);
			WriteMXFPartitionPack(pack, fpDump);
			bSeek = FALSE;		
		}

		if(!memcmp(keyValue, fillKey, sizeof(fillKey)))
			fprintf(fpDump, "[FILL]");

		if(!memcmp(keyValue, klvFillDataKey, sizeof(klvFillDataKey)))
			fprintf(fpDump, "[KLV FILL DATA]");

		if(!memcmp(keyValue, metadataKey, sizeof(metadataKey)))
		{
			fprintf(fpDump, "[METADATA] xx,yy = %02x %02x", keyValue[13], keyValue[14]);

			switch(keyValue[13]*256+keyValue[14])
			{
				case 0x012f: fprintf(fpDump, " <Preface>\r\n"); break;
				case 0x0130: fprintf(fpDump, " <Identification>\r\n"); break;
				case 0x0118: fprintf(fpDump, " <Content Storage>\r\n"); break;
				case 0x0123: fprintf(fpDump, " <Essence Container Data>\r\n"); break;
				case 0x0136: fprintf(fpDump, " <Material Package>\r\n"); break;
				case 0x0137: fprintf(fpDump, " <Source Package (File / Physical)>\r\n"); break;
				case 0x013b: fprintf(fpDump, " <Track (Timeline)>\r\n"); break;
				case 0x0139: fprintf(fpDump, " <Event Track>\r\n"); break;
				case 0x013a: fprintf(fpDump, " <Static Track>\r\n"); break;
				case 0x010f: fprintf(fpDump, " <Sequence (all cases)>\r\n"); break;
				case 0x0111: fprintf(fpDump, " <SourceClip (Picture, Sound, Data)>\r\n"); break;
				case 0x0114: fprintf(fpDump, " <Timecode Component>\r\n"); break;
				case 0x0141: fprintf(fpDump, " <DM Segment>\r\n"); break;
				case 0x0145: fprintf(fpDump, " <DM SourceClip>\r\n"); break;
				case 0x0125: fprintf(fpDump, " <File Descriptor>\r\n"); break;
				case 0x0127: fprintf(fpDump, " <Generic Picture Essence Descriptor>"); break;
				case 0x0128: fprintf(fpDump, " <CDCI Essence Descriptor>\r\n"); break;
				case 0x0129: fprintf(fpDump, " <RGBA Essence Descriptor>\r\n"); break;
				case 0x0142: fprintf(fpDump, " <Generic Sound Essence Descriptor>\r\n"); break;
				case 0x0143: fprintf(fpDump, " <Generic Data Essence Descriptor>\r\n"); break;
				case 0x0144: fprintf(fpDump, " <Multiple Descriptor>\r\n"); break;
				case 0x0132: fprintf(fpDump, " <Network Locator>\r\n"); break;
				case 0x0133: fprintf(fpDump, " <Text Locator>\r\n"); break;
				case 0x0151: fprintf(fpDump, " <MPEG2VideoDescriptor>\r\n"); break;
				case 0x0147: fprintf(fpDump, " <AES3AudioEssenceDescriptor>\r\n"); break;
				case 0x0148: fprintf(fpDump, " <WaveAudioEssenceDescriptor>\r\n"); break;
			}			
			
			if( (keyValue[13]==0x01) && (keyValue[14] == 0x2f)) // PREFACE
			{
				Preface p;
				p.Read(hMXF, size);
				WriteMXFPreface(p, fpDump);
				bSeek = FALSE;
			}
			if( (keyValue[13]==0x01) && (keyValue[14] == 0x30)) // IDENTIFICATION
			{
				Identification id;
				id.Read(hMXF, size);
				WriteMXFIdentification(id, fpDump);
				bSeek = FALSE;
			}
			if( (keyValue[13]==0x01) && (keyValue[14] == 0x18)) // CONTENT STORAGE
			{
				ContentStorage cs;
				cs.Read(hMXF, size);
				WriteMXFContentStorage(cs, fpDump);
				bSeek = FALSE;
			}
			if( (keyValue[13]==0x01) && (keyValue[14] == 0x23)) // ESSENCE CONTAINER DATA
			{
				EssenceContainerData ecd;
				ecd.Read(hMXF, size);
				WriteMXFEssenceContainerData(ecd, fpDump);
				bSeek = FALSE;
			}
			if( (keyValue[13]==0x01) && (keyValue[14] == 0x36)) // MATERIAL PACKAGE
			{
				GenericPackage gp;
				gp.Read(hMXF, size);
				WriteMXFGenericPackage(gp, fpDump);
				bSeek = FALSE;
			}
			if( (keyValue[13]==0x01) && (keyValue[14] == 0x3a)) // STATIC TRACK
			{
				StaticTrack t;
				t.Read(hMXF, size);
				WriteMXFStaticTrack(t, fpDump);
				bSeek = FALSE;
			}
			if( (keyValue[13]==0x01) && (keyValue[14] == 0x3b)) // TRACK
			{
				Track t;
				t.Read(hMXF, size);
				WriteMXFTrack(t, fpDump);
				bSeek = FALSE;
			}
			if( (keyValue[13]==0x01) && (keyValue[14] == 0x11)) // SOURCE CLIP
			{
				SourceClip clip;
				clip.Read(hMXF, size);
				WriteMXFMetadataSourceClip(clip, fpDump);
				bSeek = FALSE;
			}
			if( (keyValue[13]==0x01) && (keyValue[14] == 0x0f)) // SEQUENCE
			{
				Sequence seq;
				seq.Read(hMXF, size);
				WriteMXFMetadataSequence(seq, fpDump);
				bSeek = FALSE;
			}
			if( (keyValue[13]==0x01) && (keyValue[14] == 0x14)) // TIMECODE COMPONENT
			{
				TimecodeComponent tc;
				tc.Read(hMXF, size);
				WriteMXFMetadataTimecodeComponent(tc, fpDump);
				bSeek = FALSE;
			}
			if( (keyValue[13]==0x01) && (keyValue[14] == 0x37)) // SOURCE PACKAGE
			{
				SourcePackage sp;
				sp.Read(hMXF, size);
				WriteMXFSourcePackage(sp, fpDump);
				bSeek = FALSE;
			}
			if( (keyValue[13]==0x01) && (keyValue[14] == 0x44)) // MULTIPLE DESCRIPTOR
			{
				MultipleDescriptor md;
				md.Read(hMXF, size);
				WriteMXFMultipleDescriptor(md, fpDump);
				bSeek = FALSE;
			}
			if( (keyValue[13]==0x01) && (keyValue[14] == 0x28)) // CDCI PICTURE ESSENCE DESCRIPTOR
			{
				CDCIPictureEssenceDescriptor cped;
				cped.Read(hMXF, size);
				WriteMXFCDCIPictureEssenceDescriptor(cped, fpDump);
				bSeek = FALSE;
			}
			if( (keyValue[13]==0x01) && (keyValue[14] == 0x42)) // GENERIC SOUND ESSENCE DESCRIPTOR
			{
				GenericSoundEssenceDescriptor gsed;
				gsed.Read(hMXF, size);
				WriteMXFGenericSoundEssenceDescriptor(gsed, fpDump);
				bSeek = FALSE;
			}

			if( (keyValue[13]==0x01) && (keyValue[14] == 0x47)) // AES3 AUDIO ESSENCE DESCRIPTOR
			{
				AES3AudioEssenceDescriptor aes3AED;
				aes3AED.Read(hMXF, size);
				WriteMXFAES3AudioEssenceDescriptor(aes3AED, fpDump);
				bSeek = FALSE;
			}

			if( (keyValue[13]==0x01) && (keyValue[14] == 0x48)) // WAVE AUDIO ESSENCE DESCRIPTOR
			{
				WaveAudioEssenceDescriptor waveAED;
				waveAED.Read(hMXF, size);
				WriteMXFWaveAudioEssenceDescriptor(waveAED, fpDump);
				bSeek = FALSE;
			}

			if( (keyValue[13]==0x01) && (keyValue[14] == 0x51)) // MPEG2 VIDEO DESCRIPTOR
			{
				MPEG2VideoDescriptor mpg2VD;
				mpg2VD.Read(hMXF, &primerPack, size);
				WriteMXFMPEG2VideoDescriptor(mpg2VD, fpDump);
				bSeek = FALSE;
			}

			if( (keyValue[13]==0x01) && (keyValue[14] == 0x41)) // DM Segment
			{
				DMSegment dms;
				dms.Read(hMXF, size);
				WriteMXFDMSegment(dms, fpDump);
				bSeek = FALSE;
			}			
		}

		if(!memcmp(keyValue, indexTableKey, sizeof(indexTableKey)))
		{
			fprintf(fpDump, "[INDEX TABLE]\r\n");
			IndexTableSegment its;
			its.Read(hMXF, size);
			WriteMXFIndexTable(its, fpDump);
			bSeek = FALSE;
		}
		
		if(!memcmp(keyValue, systemMetadataPackKey, sizeof(systemMetadataPackKey)))
		{
			fprintf(fpDump, "[SYSTEM METADATA PACK]\r\n");
			SystemMetadataPack smp;
			smp.Read(hMXF);
			WriteMXFSystemMetadataPack(smp, fpDump);
			bSeek = FALSE;
		}

		if(!memcmp(keyValue, packageMetadataSetKey, sizeof(packageMetadataSetKey)))
		{
			int blockCount = keyValue[15];
			fprintf(fpDump, "[PACKAGE METADATA SET] => MetadataBlockCount = %d", blockCount);
		}

		if(!memcmp(keyValue, xmlDocumentTextKey, sizeof(xmlDocumentTextKey)))
		{
			fprintf(fpDump, "[XMLDocument]");

			fprintf(fpDump, "\r\n");
			for(unsigned int i = 0; i < size; i++)
			{
				char ch;
				DWORD nRead;
				ReadFile(hMXF, &ch, 1, &nRead, NULL);
				fprintf(fpDump, "%c", ch);
			}
			fprintf(fpDump, "\r\n");
			bSeek = false;
		}
		
		if(!memcmp(keyValue, randomIndexPackKey, sizeof(randomIndexPackKey)))
		{
			fprintf(fpDump, "[RANDOM INDEX PACK]\r\n");

			RandomIndexPack rip;
			rip.Read(hMXF, size);
			WriteMXFRandomIndexPack(rip, fpDump);
		}

		fprintf(fpDump, "\r\n");

		if(bSeek)
		{
			LARGE_INTEGER pos = {0};
			DWORD ret;

			pos.QuadPart = size;
			ret = SetFilePointer(hMXF, pos.LowPart, &pos.HighPart, FILE_CURRENT);
			if(ret == INVALID_SET_FILE_POINTER && GetLastError() != ERROR_SUCCESS)
				break;
		}
	}

	fclose(fpDump);

	return TRUE;
}


void MXFLogger::WriteMXFPartitionPack(PartitionPack& ppack, FILE* fpDump)
{
	// Dump do header no prompt
	fprintf(fpDump, "\tMajor Version = %d\r\n", ppack.majorVersion);
	fprintf(fpDump, "\tMinor Version = %d\r\n", ppack.minorVersion); ;
	fprintf(fpDump, "\tKAG Size = %d\r\n", ppack.kagSize); ;
	fprintf(fpDump, "\tThis Partition = 0x%08x\r\n", ppack.thisPartition); ;
	fprintf(fpDump, "\tPrevious Partition = 0x%08x\r\n", ppack.previousPartition); ;
	fprintf(fpDump, "\tFooter Partition = 0x%08x\r\n", ppack.footerPartition); ;
	fprintf(fpDump, "\tHeader Byte Count = %d\r\n", ppack.headerByteCount); ;
	fprintf(fpDump, "\tIndexByteCount = %d\r\n", ppack.indexByteCount); ;
	fprintf(fpDump, "\tIndexSID = %d\r\n", ppack.indexSID); ;
	fprintf(fpDump, "\tBodyOffset = %d\r\n", ppack.bodyOffset); ;
	fprintf(fpDump, "\tBodySID = %d\r\n", ppack.bodySID); ;
	fprintf(fpDump, "\tOperational Pattern  = ");
	for(int i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", ppack.operationPattern[i]); ;
	}
	fprintf(fpDump, "\r\n"); ;
	fprintf(fpDump, "\tEssence Containers: "); ;
	for(unsigned int i = 0; i < ppack.essenceContainers.num; i++)
	{
		for(unsigned int j = 0; j < ppack.essenceContainers.size; j++)
		{
			fprintf(fpDump, "%02x ", ppack.essenceContainers.ppItem[i][j]); ;
		}
		fprintf(fpDump, "\r\n\t"); ;
	}
	fprintf(fpDump, "\r\n");
}

void MXFLogger::WriteMXFPrimerPack(PrimerPack& pp, FILE* fpDump)
{
	unsigned int i, j;
	fprintf(fpDump, "\tNumberOfItems = %d\r\n", pp.localTagEntryBatch.numberOfItems);
	
	fprintf(fpDump, "\tItemLength = %d\r\n", pp.localTagEntryBatch.itemLength);
	
	for(i = 0; i < pp.localTagEntryBatch.numberOfItems; i++)
	{
		fprintf(fpDump, "\t\t0x%04x => ", pp.localTagEntryBatch.pItems[i].localTag);
	
		for(j = 0; j < 16; j++)
			fprintf(fpDump, "%02x ", pp.localTagEntryBatch.pItems[i].uid[j]);
		
		fprintf(fpDump, "\r\n");	
	}
}

void MXFLogger::WriteMXFIndexTable(IndexTableSegment& its, FILE* fpDump)
{
	unsigned int i;

	fprintf(fpDump, "\tInstanceUID = ");
	for(i = 0; i < 16; i++)
		fprintf(fpDump, "%02x ", its.instanceID[i]);
	fprintf(fpDump, "\r\n");

	fprintf(fpDump, "\tIndexEditRate = %d/%d\r\n", its.indexEditRate.num, its.indexEditRate.den);
	fprintf(fpDump, "\tIndexStartPosition = %I64d\r\n", its.indexStartPosition);
	fprintf(fpDump, "\tIndexDuration = %I64d\r\n", its.indexDuration);

	if(its.bEditUnitByteCount)
		fprintf(fpDump, "\tEditUnitByteCount = %d\r\n", its.editUnitByteCount);
	else
		fprintf(fpDump, "\tEditUnitByteCount = (encoder optional and not specified)\r\n");
	
	if(its.bIndexSID)
		fprintf(fpDump, "\tIndexSID = %d\r\n", its.indexSID);
	else
		fprintf(fpDump, "\tIndexSID = (encoder optional and not specified)\r\n");
	
	fprintf(fpDump, "\tBodySID = %d\r\n", its.bodySID);
	
	if(its.bSliceCount)
	{
		fprintf(fpDump, "\tSliceCount = %d\r\n", its.sliceCount);	
	}
	else
	{
		fprintf(fpDump, "\tSliceCount = (encoder optional and not specified)\r\n");
	}

	if(its.bPosTableCount)
	{
		fprintf(fpDump, "\tPosTableCount = %d\r\n", its.posTableCount);		
	}
	else
	{
		fprintf(fpDump, "\tPosTableCount = (optional and not specified)\r\n");		
	}

	if(its.bDeltaEntryArray)
	{
		fprintf(fpDump, "\tDeltaEntryArray:\r\n");		
		fprintf(fpDump, "\t\tNDE = %d\r\n", its.deltaEntryArray.NDE);
		fprintf(fpDump, "\t\tLength = %d\r\n", its.deltaEntryArray.length);
		
		fprintf(fpDump, "\t\t-------------------------\r\n");		
		for(unsigned int i = 0; i < its.deltaEntryArray.NDE; i++)
		{
			fprintf(fpDump, "\t\tPosTableIndex = %d\r\n", its.deltaEntryArray.pDeltaEntry[i].posTableIndex);
			fprintf(fpDump, "\t\tSlice = %d\r\n", its.deltaEntryArray.pDeltaEntry[i].slice);
			fprintf(fpDump, "\t\tElementDelta = %d\r\n", its.deltaEntryArray.pDeltaEntry[i].elementDelta);
			fprintf(fpDump, "\t\t-------------------------\r\n");			
		}
	}
	else
	{
		fprintf(fpDump, "\tDeltaEntryArray = (optional and not specified)\r\n");
	}

	if(its.bIndexEntryArray)
	{
		fprintf(fpDump, "\tIndexEntryArray:\r\n");
		fprintf(fpDump, "\t\tNDE = %d\r\n", its.indexEntryArray.NIE);
		fprintf(fpDump, "\t\tLength = %d\r\n", its.indexEntryArray.length);
	
		fprintf(fpDump, "\t\t-------------------------\r\n");
		for(unsigned int i = 0; i < its.indexEntryArray.NIE; i++)
		{
			fprintf(fpDump, "\t\tTemporalOffset = %d\r\n", its.indexEntryArray.pIndexEntry[i].temporalOffset);
			fprintf(fpDump, "\t\tKeyFrameOffset = %d\r\n", its.indexEntryArray.pIndexEntry[i].keyFrameOffset);
			fprintf(fpDump, "\t\tFlags = %d\r\n", its.indexEntryArray.pIndexEntry[i].flags);
			fprintf(fpDump, "\t\tStreamOffset = %I64d\r\n", its.indexEntryArray.pIndexEntry[i].streamOffset);
	
			if(its.bSliceCount && (its.sliceCount > 0))
			{
				for(unsigned int j=0; j < its.sliceCount; j++)
				{
					fprintf(fpDump, "\t\tSliceOffset (%d) = %d\r\n", j, its.indexEntryArray.pIndexEntry[i].pSliceOffset[j]);			
				}
			}
			if(its.bPosTableCount && (its.posTableCount > 0))
			{
				for(unsigned int j=0; j < its.sliceCount; j++)
				{
					fprintf(fpDump, "\t\tPosTableCount (%d) = %d/%d\r\n", j, its.indexEntryArray.pIndexEntry[i].pPosTable[j].num, its.indexEntryArray.pIndexEntry[i].pPosTable[j].den);					
				}
			}
			fprintf(fpDump, "\t\t-------------------------\r\n");			
		}
	}
	else
	{
		fprintf(fpDump, "\tIndexEntryArray = (encoder optional and not specified)\r\n");		
	}
}

void MXFLogger::WriteMXFPreface(Preface& p, FILE* fpDump)
{
	unsigned int i, j;

	fprintf(fpDump, "\tInstanceUID = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", p.instanceUID[i]);
	}
	fprintf(fpDump, "\r\n");
	
	if(p.bGenerationUID)
	{
		fprintf(fpDump, "\tGenerationUID = ");
		for(i = 0; i < 16; i++)
		{
			fprintf(fpDump, "%02x ", p.generationUID[i]);
		}
		fprintf(fpDump, "\r\n");	
	}
	else
	{
		fprintf(fpDump, "\tGenerationUID = (optional and not specified)\r\n");		
	}

	fprintf(fpDump, "\tLastModifiedDate = %02d/%02d/%04d - %02d:%02d:%02d-%02dms/4\r\n", p.lastModifiedDate.st.day, p.lastModifiedDate.st.month, p.lastModifiedDate.st.year, p.lastModifiedDate.st.hour, p.lastModifiedDate.st.minute, p.lastModifiedDate.st.second, p.lastModifiedDate.st.quartermsec);
	fprintf(fpDump, "\tVersion = %d\r\n", p.version);

	if(p.bObjectModelVersion)
	{
		fprintf(fpDump, "\tObjectModelVersion = %d\r\n", p.objectModelVersion);
	}
	else
	{
		fprintf(fpDump, "\tObjectModelVersion = (optional and not specified)\r\n");
	}

	if(p.bPrimaryPackage)
	{
		fprintf(fpDump, "\tPrimaryPackage = ");
		for(i = 0; i < 16; i++)
		{
			fprintf(fpDump, "%02x ", p.primaryPackage[i]);
		}
		fprintf(fpDump, "\r\n");
	}
	else
	{
		fprintf(fpDump, "\tPrimary Package = (optional and not specified)\r\n");
	}

	fprintf(fpDump, "\tIdentifications:\r\n");
	fprintf(fpDump, "\t\tNum = %d\r\n", p.identifications.num);
	fprintf(fpDump, "\t\tSize = %d\r\n", p.identifications.size);

	for(i = 0; i < p.identifications.num; i++)
	{
		fprintf(fpDump, "\t\t");
		for(j = 0; j < p.identifications.size; j++)
		{
			fprintf(fpDump, "%02x ", p.identifications.ppItem[i][j]);
		}
		fprintf(fpDump, "\r\n");
	}

	fprintf(fpDump, "\tContentStorage = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", p.contentStorage[i]);
	}
	fprintf(fpDump, "\r\n");

	fprintf(fpDump, "\tOperational Pattern = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", p.operationalPattern[i]);
	}
	fprintf(fpDump, "\r\n");
	fprintf(fpDump, "\tEssence Containers:\r\n");
	fprintf(fpDump, "\t\tNum = %d\r\n", p.essenceContainers.num);
	fprintf(fpDump, "\t\tSize = %d\r\n", p.essenceContainers.size);

	for(i = 0; i < p.essenceContainers.num; i++)
	{
		fprintf(fpDump, "\t\t");
		for(j = 0; j < p.essenceContainers.size; j++)
		{
			fprintf(fpDump, "%02x ", p.essenceContainers.ppItem[i][j]);
		}
		fprintf(fpDump, "\r\n");
	}

	fprintf(fpDump, "\tDM Schemes:\r\n");
	fprintf(fpDump, "\t\tNum = %d\r\n", p.DMSchemes.num);
	fprintf(fpDump, "\t\tSize = %d\r\n", p.DMSchemes.size);

	for(i = 0; i < p.DMSchemes.num; i++)
	{
		fprintf(fpDump, "\t\t");
		for(j = 0; j < p.DMSchemes.size; j++)
		{
			fprintf(fpDump, "%02x ", p.DMSchemes.ppItem[i][j]);
		}
		fprintf(fpDump, "\r\n");
	}
}

void MXFLogger::WriteMXFIdentification(Identification& id, FILE* fpDump)
{
	unsigned int i;

	fprintf(fpDump, "\tInstanceUID = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", id.instanceUID[i]);
	}
	fprintf(fpDump, "\r\n");
	
	fprintf(fpDump, "\tThisGenerationUID = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", id.thisGenerationUID[i]);
	}
	fprintf(fpDump, "\r\n");
	
	fprintf(fpDump, "\tCompany Name = %S\r\n", id.companyName);
	fprintf(fpDump, "\tProduct Name = %S\r\n", id.productName);
	
	if(id.bProductVersion)
	{
		fprintf(fpDump, "\tProduct Version = %d.%d.%d.%d.%d\r\n", id.productVersion[0], id.productVersion[1], id.productVersion[2], id.productVersion[3], id.productVersion[4]);
	}
	else
	{
		fprintf(fpDump, "\tProduct Version = (optional and not specified)\r\n");
	}

	fprintf(fpDump, "\tVersion String = %S\r\n", id.versionString);
	fprintf(fpDump, "\tProduct UID = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", id.productUID[i]);
	}
	fprintf(fpDump, "\r\n");
	
	fprintf(fpDump,"\tModificationDate = %02d/%02d/%04d - %02d:%02d:%02d-%02dms/4\r\n", id.modificationDate.st.day, id.modificationDate.st.month, id.modificationDate.st.year, id.modificationDate.st.hour, id.modificationDate.st.minute, id.modificationDate.st.second, id.modificationDate.st.quartermsec);
	
	if(id.bToolkitVersion)
	{
		fprintf(fpDump, "\tToolkit Version = %d.%d.%d.%d.%d\r\n", id.toolkitVersion[0], id.toolkitVersion[1], id.toolkitVersion[2], id.toolkitVersion[3], id.toolkitVersion[4]);
	}
	else
	{
		fprintf(fpDump, "\tToolkit Version = (optional and not specified)\r\n");
	}

	if(id.bPlatform)
	{
		fprintf(fpDump, "\tPlatform = %S\r\n", id.platform);
	}
	else
	{
		fprintf(fpDump, "\tPlatform = (optional and not specified)\r\n");
	}
}

void MXFLogger::WriteMXFContentStorage(ContentStorage& cs, FILE* fpDump)
{
	unsigned int i, j;

	fprintf(fpDump, "\tInstanceUID = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", cs.instanceUID[i]);
	}
	fprintf(fpDump, "\r\n");
	
	if(cs.bGenerationUID)
	{
		fprintf(fpDump, "\tGenerationUID = ");
		for(i = 0; i < 16; i++)
		{
			fprintf(fpDump, "%02x ", cs.generationUID[i]);
		}
		fprintf(fpDump, "\r\n");
	}
	else
	{
		fprintf(fpDump, "\tGenerationUID = (optional and not specified)\r\n");
	}
	fprintf(fpDump, "\tPackages:\r\n");
	fprintf(fpDump, "\t\tNum = %d\r\n", cs.packages.num);
	fprintf(fpDump, "\t\tSize = %d\r\n", cs.packages.size);

	for(i = 0; i < cs.packages.num; i++)
	{
		fprintf(fpDump, "\t\t");
		for(j = 0; j < cs.packages.size; j++)
		{
			fprintf(fpDump, "%02x ", cs.packages.ppItem[i][j]);
		}
		fprintf(fpDump, "\r\n");
	}

	if(cs.bEssenceContainerData)
	{
		fprintf(fpDump, "\tEssence Container Data:\r\n");
		fprintf(fpDump, "\t\tNum = %d\r\n", cs.essenceContainerData.num);
		fprintf(fpDump, "\t\tSize = %d\r\n", cs.essenceContainerData.size);

		for(i = 0; i < cs.essenceContainerData.num; i++)
		{
			fprintf(fpDump, "\t\t");
			for(j = 0; j < cs.essenceContainerData.size; j++)
			{
				fprintf(fpDump, "%02x ", cs.essenceContainerData.ppItem[i][j]);
			}
			fprintf(fpDump, "\r\n");
		}
	}
	else
	{
		fprintf(fpDump, "\tEssence Container Data = (optional and not specified)\r\n");
	}
}

void MXFLogger::WriteMXFEssenceContainerData(EssenceContainerData& ecd, FILE* fpDump)
{
	unsigned int i;

	fprintf(fpDump, "\tInstanceUID = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", ecd.instanceUID[i]);
	}
	fprintf(fpDump, "\r\n");
	
	fprintf(fpDump, "\tLinked Package UID = ");
	for(i = 0; i < 32; i++)
	{
		fprintf(fpDump, "%02x ", ecd.linkedPackageUID[i]);
	}
	fprintf(fpDump, "\r\n");
	
	if(ecd.bGenerationUID)
	{
		fprintf(fpDump, "\tGenerationUID = ");
		for(i = 0; i < 16; i++)
		{
			fprintf(fpDump, "%02x ", ecd.generationUID[i]);
		}
		fprintf(fpDump, "\r\n");
	}
	else
	{
		fprintf(fpDump, "\tGenerationUID = (optional and not specified)\r\n");
	}

	if(ecd.bIndexSID)
	{
		fprintf(fpDump, "\tIndexSID = %d\r\n", ecd.indexSID);
	}
	else
	{
		fprintf(fpDump, "\tIndexSID = (optional and not specified)\r\n");
	}

	fprintf(fpDump, "\tBodySID = %d\r\n", ecd.bodySID);	
}

void MXFLogger::WriteMXFGenericPackage(GenericPackage& gp, FILE* fpDump)
{
	unsigned int i, j;

	fprintf(fpDump, "\tInstanceUID = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", gp.instanceUID[i]);
	}
	fprintf(fpDump, "\r\n");
	
	fprintf(fpDump, "\tPackage UID = ");
	for(i = 0; i < 32; i++)
	{
		fprintf(fpDump, "%02x ", gp.packageUID[i]);
	}
	fprintf(fpDump, "\r\n");
	
	if(gp.bGenerationUID)
	{
		fprintf(fpDump, "\tGenerationUID = ");
		for(i = 0; i < 16; i++)
		{
			fprintf(fpDump, "%02x ", gp.generationUID[i]);
		}
		fprintf(fpDump, "\r\n");
	}
	else
	{
		fprintf(fpDump, "\tGenerationUID = (optional and not specified)\r\n");
	}

	if(gp.bName)
	{
		fprintf(fpDump, "\tName = %S\r\n", gp.name);
	}
	else
	{
		fprintf(fpDump, "\tName = (optional and not specified)\r\n");
	}

	fprintf(fpDump,"\tPackageCreationDate = %02d/%02d/%04d - %02d:%02d:%02d-%02dms/4\r\n", gp.packageCreationDate.st.day, gp.packageCreationDate.st.month, gp.packageCreationDate.st.year, gp.packageCreationDate.st.hour, gp.packageCreationDate.st.minute, gp.packageCreationDate.st.second, gp.packageCreationDate.st.quartermsec);
	
	fprintf(fpDump,"\tPackageModifiedDate = %02d/%02d/%04d - %02d:%02d:%02d-%02dms/4\r\n", gp.packageModifiedDate.st.day, gp.packageModifiedDate.st.month, gp.packageModifiedDate.st.year, gp.packageModifiedDate.st.hour, gp.packageModifiedDate.st.minute, gp.packageModifiedDate.st.second, gp.packageModifiedDate.st.quartermsec);
	
	fprintf(fpDump, "\tTracks:\r\n");
	fprintf(fpDump, "\t\tNum = %d\r\n", gp.tracks.num);
	fprintf(fpDump, "\t\tSize = %d\r\n", gp.tracks.size);

	for(i = 0; i < gp.tracks.num; i++)
	{
		fprintf(fpDump, "\t\t");
		for(j = 0; j < gp.tracks.size; j++)
		{
			fprintf(fpDump, "%02x ", gp.tracks.ppItem[i][j]);
		}
		fprintf(fpDump, "\r\n");
	}

	fprintf(fpDump, "\tDescriptor = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", gp.descriptor[i]);
	}
	fprintf(fpDump, "\r\n");
}

void MXFLogger::WriteMXFTrack(Track& t, FILE* fpDump)
{
	unsigned int i;

	fprintf(fpDump, "\tInstanceUID = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", t.instanceUID[i]);
	}
	fprintf(fpDump, "\r\n");
	
	if(t.bGenerationUID)
	{
		fprintf(fpDump, "\tGenerationUID = ");
		for(i = 0; i < 16; i++)
		{
			fprintf(fpDump, "%02x ", t.generationUID[i]);
		}
		fprintf(fpDump, "\r\n");
	}
	else
	{
		fprintf(fpDump, "\tGenerationUID = (optional and not specified)\r\n");
	}

	if(t.bTrackID)
	{
		fprintf(fpDump, "\tTrackID = %d\r\n", t.trackID);
	}
	else
	{
		fprintf(fpDump, "\tTrackID = (encoder optional and not specified)\r\n");
	}

	fprintf(fpDump, "\tTrackNumber = 0x%08x\r\n", t.trackNumber);
	
	if(t.bTrackName)
	{
		fprintf(fpDump, "\tTrackName = %S\r\n", t.trackName);
	}
	else
	{
		fprintf(fpDump, "\tTrackName = (optional and not specified)\r\n");
	}

	fprintf(fpDump, "\tEditRate = %d/%d\r\n", t.editRate.num, t.editRate.den);
	fprintf(fpDump, "\tOrigin = %I64d\r\n", t.origin);
	fprintf(fpDump, "\tSequence = ");

	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", t.sequence[i]);
	}
	fprintf(fpDump, "\r\n");
}

void MXFLogger::WriteMXFStaticTrack(StaticTrack& t, FILE* fpDump)
{
	unsigned int i;

	fprintf(fpDump, "\tInstanceUID = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", t.instanceUID[i]);
	}
	fprintf(fpDump, "\r\n");

	if(t.bGenerationUID)
	{
		fprintf(fpDump, "\tGenerationUID = ");
		for(i = 0; i < 16; i++)
		{
			fprintf(fpDump, "%02x ", t.generationUID[i]);
		}
		fprintf(fpDump, "\r\n");
	}
	else
	{
		fprintf(fpDump, "\tGenerationUID = (optional and not specified)\r\n");
	}

	if(t.bTrackID)
	{
		fprintf(fpDump, "\tTrackID = %d\r\n", t.trackID);
	}
	else
	{
		fprintf(fpDump, "\tTrackID = (encoder optional and not specified)\r\n");
	}

	fprintf(fpDump, "\tTrackNumber = 0x%08x\r\n", t.trackNumber);

	if(t.bTrackName)
	{
		fprintf(fpDump, "\tTrackName = %S\r\n", t.trackName);
	}
	else
	{
		fprintf(fpDump, "\tTrackName = (optional and not specified)\r\n");
	}

	fprintf(fpDump, "\tSequence = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", t.sequence[i]);	
	}
	fprintf(fpDump, "\r\n");	
}

void MXFLogger::WriteMXFMetadataSourceClip(SourceClip& clip, FILE* fpDump)
{
	unsigned int i;

	fprintf(fpDump, "\tInstanceUID = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", clip.instanceUID[i]);
	}
	fprintf(fpDump, "\r\n");

	if(clip.bGenerationUID)
	{
		fprintf(fpDump, "\tGenerationUID = ");
		for(i = 0; i < 16; i++)
		{
			fprintf(fpDump, "%02x ", clip.generationUID[i]);
		}
		fprintf(fpDump, "\r\n");
	}
	else
	{
		fprintf(fpDump, "\tGenerationUID = (optional and not specified)\r\n");
	}

	fprintf(fpDump, "\tData Definition = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", clip.dataDefinition[i]);
	}
	fprintf(fpDump, "\r\n");
	fprintf(fpDump, "\tStart Position = %I64d\r\n", clip.startPosition);
	fprintf(fpDump, "\tDuration = %I64d\r\n", clip.duration);

	if(clip.bSourcePackageID)
	{
		fprintf(fpDump, "\tSource Package ID = ");
		for(i = 0; i < 32; i++)
		{
			fprintf(fpDump, "%02x ", clip.sourcePackageID[i]);
		}
		fprintf(fpDump, "\r\n");
	}
	else
	{
		fprintf(fpDump, "\tSource Package ID = (optional and not specified)\r\n");
	}

	fprintf(fpDump, "\tSource Track ID = %d", clip.sourceTrackID);
}

void MXFLogger::WriteMXFMetadataSequence(Sequence& seq, FILE* fpDump)
{
	unsigned int i, j;
	fprintf(fpDump, "\tInstanceUID = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", seq.instanceUID[i]);
	}
	fprintf(fpDump, "\r\n");
	
	if(seq.bGenerationUID)
	{
		fprintf(fpDump, "\tGenerationUID = ");
		for(i = 0; i < 16; i++)
		{
			fprintf(fpDump, "%02x ", seq.generationUID[i]);
		}
		fprintf(fpDump, "\r\n");
	}
	else
	{
		fprintf(fpDump, "\tGenerationUID = (optional and not specified)\r\n");
	}

	fprintf(fpDump, "\tData Definition = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", seq.dataDefinition[i]);
	}
	fprintf(fpDump, "\r\n");
	fprintf(fpDump, "\tDuration = %I64d\r\n", seq.duration);
	fprintf(fpDump, "\tStructural Components:\r\n");
	fprintf(fpDump, "\t\tNum = %d\r\n", seq.structuralComponents.num);
	fprintf(fpDump, "\t\tSize = %d\r\n", seq.structuralComponents.size);

	for(i = 0; i < seq.structuralComponents.num; i++)
	{
		fprintf(fpDump, "\t\t");
		for(j = 0; j < seq.structuralComponents.size; j++)
		{
			fprintf(fpDump, "%02x ", seq.structuralComponents.ppItem[i][j]);
		}
		fprintf(fpDump, "\r\n");
	}
}

void MXFLogger::WriteMXFMetadataTimecodeComponent(TimecodeComponent tc, FILE* fpDump)
{
	unsigned int i;
	fprintf(fpDump, "\tInstanceUID = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", tc.instanceUID[i]);
	}
	fprintf(fpDump, "\r\n");
	
	if(tc.bGenerationUID)
	{
		fprintf(fpDump, "\tGenerationUID = ");
		for(i = 0; i < 16; i++)
		{
			fprintf(fpDump, "%02x ", tc.generationUID[i]);
		}
		fprintf(fpDump, "\r\n");
	}
	else
	{
		fprintf(fpDump, "\tGenerationUID = (optional and not specified)\r\n");
	}

	fprintf(fpDump, "\tData Definition = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", tc.dataDefinition[i]);
	}
	fprintf(fpDump, "\r\n");
	
	fprintf(fpDump, "\tDuration = %I64d\r\n", tc.duration);
	fprintf(fpDump, "\tRounded Timecode Base= %d\r\n", tc.roundedTimecodeBase);
	fprintf(fpDump, "\tStartTimecode = %I64d\r\n", tc.startTimecode);
	fprintf(fpDump, "\tDropFrame = %d\r\n", tc.dropFrame);
}

void MXFLogger::WriteMXFSourcePackage(SourcePackage& sp, FILE* fpDump)
{	
	WriteMXFGenericPackage(sp.package, fpDump);
}

void MXFLogger::WriteMXFFileDescriptor(FileDescriptor& fd, FILE* fpDump)
{
	unsigned int i, j;
	fprintf(fpDump, "\tInstanceUID = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", fd.instanceUID[i]);
	}
	fprintf(fpDump, "\r\n");
	
	if(fd.bGenerationUID)
	{
		fprintf(fpDump, "\tGenerationUID = ");
		for(i = 0; i < 16; i++)
		{
			fprintf(fpDump, "%02x ", fd.generationUID[i]);
		}
		fprintf(fpDump, "\r\n");
	}
	else
	{
		fprintf(fpDump, "\tGenerationUID = (optional and not specified)\r\n");
	}

	if(fd.bLinkedTrackID)
	{
		fprintf(fpDump, "\tLinkedTrackID = %d\r\n", fd.linkedTrackID);
	}
	else
	{
		fprintf(fpDump, "\tLinkedTrackID = (optional and not specified)\r\n");
	}

	fprintf(fpDump, "\tSampleRate = %d/%d\r\n", fd.sampleRate.num, fd.sampleRate.den);
	
	if(fd.bContainerDuration)
	{
		fprintf(fpDump, "\tContainerDuration = %I64d\r\n", fd.containerDuration);
	}
	else
	{
		fprintf(fpDump, "\tContainerDuration = (optional and not specified)\r\n");
	}

	fprintf(fpDump, "\tEssenceContainer = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", fd.essenceContainer[i]);
	}
	fprintf(fpDump, "\r\n");
	
	if(fd.bCodec)
	{
		fprintf(fpDump, "\tCodec = ");
		for(i = 0; i < 16; i++)
		{
			fprintf(fpDump, "%02x ", fd.codec[i]);
		}
		fprintf(fpDump, "\r\n");
	}
	else
	{
		fprintf(fpDump, "\tCodec = (optional and not specified)\r\n");
	}

	if(fd.bLocators)
	{
		fprintf(fpDump, "\tLocators:\r\n");
		fprintf(fpDump, "\t\tNum = %d\r\n", fd.locators.num);
		fprintf(fpDump, "\t\tSize = %d\r\n", fd.locators.size);

		for(i = 0; i < fd.locators.num; i++)
		{
			fprintf(fpDump, "\t\t");
			for(j = 0; j < fd.locators.size; j++)
			{
				fprintf(fpDump, "%02x ", fd.locators.ppItem[i][j]);
			}
			fprintf(fpDump, "\r\n");
		}
	}
	else
	{
		fprintf(fpDump, "\tLocators = (optional and not specified)\r\n");		
	}
}

void MXFLogger::WriteMXFMultipleDescriptor(MultipleDescriptor& md, FILE* fpDump)
{
	unsigned int i, j;
	WriteMXFFileDescriptor(md.fileDescriptor, fpDump);

	fprintf(fpDump, "\tSubDescriptorUIDs:\r\n");
	fprintf(fpDump, "\t\tNum = %d\r\n", md.subDescriptorUIDs.num);
	fprintf(fpDump, "\t\tSize = %d\r\n", md.subDescriptorUIDs.size);

	for(i = 0; i < md.subDescriptorUIDs.num; i++)
	{
		fprintf(fpDump, "\t\t");
		for(j = 0; j < md.subDescriptorUIDs.size; j++)
		{
			fprintf(fpDump, "%02x ", md.subDescriptorUIDs.ppItem[i][j]);
		}
		fprintf(fpDump, "\r\n");
	}
}

void MXFLogger::WriteMXFGenericPictureEssenceDescriptor(GenericPictureEssenceDescriptor& gped, FILE* fpDump)
{
	unsigned int i;
	char str[1023];

	WriteMXFFileDescriptor(gped.fileDescriptor, fpDump);

	if(gped.bSignalStandard)
	{
		switch(gped.signalStandard)
		{
			case 0x00: sprintf(str, "Signal Standard = 0x00 (No Specific Underlying Standard)"); break;
			case 0x01: sprintf(str, "Signal Standard = 0x01 (ITU-R BT.601 and BT.656, also SMPTE 125M (525 and 625 line interlaced))"); break;
			case 0x02: sprintf(str, "Signal Standard = 0x02 (ITU-R BT.1358, also SMPTE 293M (525 and 625 line progressive))"); break;
			case 0x03: sprintf(str, "Signal Standard = 0x03 (SMPTE 347M (540 Mbps mappings))"); break;
			case 0x04: sprintf(str, "Signal Standard = 0x04 (SMPTE 274M (1125 line))"); break;
			case 0x05: sprintf(str, "Signal Standard = 0x05 (SMPTE 296M (750 line progressive))"); break;
			case 0x06: sprintf(str, "Signal Standard = 0x06 (SMPTE 349M (1485 Mbps mappings))"); break;
			default:
				sprintf(str, "Signal Standard = %d (desconhecido)", gped.signalStandard); break;
		}
		fprintf(fpDump, "\t%s\r\n", str);
	}
	else
	{
		fprintf(fpDump, "\tSignal Standard = (optional and not specified)\r\n");
	}

	switch(gped.frameLayout)
	{
		case 0: sprintf(str, "Frame Layout = 0 (Full Frame)"); break;
		case 1: sprintf(str, "Frame Layout = 1 (Separate Fields)"); break;
		case 2: sprintf(str, "Frame Layout = 2 (Single Field)"); break;
		case 3: sprintf(str, "Frame Layout = 3 (Mixed Fields)"); break;
		case 4: sprintf(str, "Frame Layout = 4 (Segmented Frame)"); break;
		default:
			sprintf(str, "Frame Layout = %d (desconhecido)", gped.frameLayout);
	}
	fprintf(fpDump, "\t%s\r\n", str);
	
	fprintf(fpDump, "\tStoredWidth = %d\r\n", gped.storedWidth);
	fprintf(fpDump, "\tStoredHeight = %d\r\n", gped.storedHeight);
	
	if(gped.bStoredF2Offset)
	{
		fprintf(fpDump, "\tStoredF2Offset = %d\r\n", gped.storedF2Offset);
	}
	else
	{
		fprintf(fpDump, "\tStoredF2Offset = (optional and not specified)\r\n");
	}

	if(gped.bSampledWidth)
	{
		fprintf(fpDump, "\tSampledWidth = %d\r\n", gped.sampledWidth);
	}
	else
	{
		fprintf(fpDump, "\tSampledWidth = (optional and not specified)\r\n");
	}

	if(gped.bSampledHeight)
	{
		fprintf(fpDump, "\tSampledHeight = %d\r\n", gped.sampledHeight);
	}
	else
	{
		fprintf(fpDump, "\tSampledHeight = (optional and not specified)\r\n");
	}

	if(gped.bSampledXOffset)
	{
		fprintf(fpDump, "\tSampledXOffset = %d\r\n", gped.sampledXOffset);
	}
	else
	{
		fprintf(fpDump, "\tSampledXOffset = (optional and not specified)\r\n");
	}

	if(gped.bSampledYOffset)
	{
		fprintf(fpDump, "\tSampledYOffset = %d\r\n", gped.sampledYOffset);
	}
	else
	{
		fprintf(fpDump, "\tSampledYOffset = (optional and not specified)\r\n");
	}

	if(gped.bDisplayHeight)
	{
		fprintf(fpDump, "\tDisplayHeight = %d\r\n", gped.displayHeight);
	}
	else
	{
		fprintf(fpDump, "\tDisplayHeight = (optional and not specified)\r\n");
	}

	if(gped.bDisplayWidth)
	{
		fprintf(fpDump, "\tDisplayWidth = %d\r\n", gped.displayWidth);
	}
	else
	{
		fprintf(fpDump, "\tDisplayWidth = (optional and not specified)\r\n");
	}

	if(gped.bDisplayXOffset)
	{
		fprintf(fpDump, "\tDisplayXOffset = %d\r\n", gped.displayXOffset);
	}
	else
	{
		fprintf(fpDump, "\tDisplayXOffset = (optional and not specified)\r\n");
	}

	if(gped.bDisplayYOffset)
	{
		fprintf(fpDump, "\tDisplayYOffset = %d\r\n", gped.displayYOffset);
	}
	else
	{
		fprintf(fpDump, "\tDisplayYOffset = (optional and not specified)\r\n");
	}

	if(gped.bDisplayF2Offset)
	{
		fprintf(fpDump, "\tDisplayF2Offset = %d\r\n", gped.displayF2Offset);
	}
	else
	{
		fprintf(fpDump, "\tDisplayF2Offset = (optional and not specified)\r\n");
	}

	fprintf(fpDump, "\tAspectRatio = %d/%d\r\n", gped.aspectRatio.num, gped.aspectRatio.den);
	
	if(gped.bActiveFormatDescriptor)
	{
		fprintf(fpDump, "\tActiveFormatDescriptor = %d\r\n", gped.activeFormatDescriptor);
	}
	else
	{
		fprintf(fpDump, "\tActiveFormatDescriptor = (optional and not specified)\r\n");
	}

	fprintf(fpDump, "\tVideoLineMap = {%d, %d}\r\n", gped.videoLineMap[0], gped.videoLineMap[1]);

	if(gped.bAlphaTransparency)
	{
		fprintf(fpDump, "\tAlphaTransparency = %d\r\n", gped.alphaTransparency);
	}
	else
	{
		fprintf(fpDump, "\tAlphaTransparency = (optional and not specified)\r\n");
	}

	if(gped.bCaptureGamma)
	{
		fprintf(fpDump, "\tCaptureGamma = ");
		for(i = 0; i < 16; i++)
		{
			fprintf(fpDump, "%02x ", gped.captureGamma[i]);
		}
		fprintf(fpDump, " (ver SMPTE 268, tabela 5)\r\n");
	}
	else
	{
		fprintf(fpDump, "\tCaptureGamma = (optional and not specified)\r\n");
	}

	if(gped.bImageAlignmentOffset)
	{
		fprintf(fpDump, "\tImageAlignmentOffset = %d\r\n", gped.imageAlignmentOffset);
	}
	else
	{
		fprintf(fpDump, "\tImageAlignmentOffset = (optional and not specified)\r\n");
	}

	if(gped.bImageStartOffset)
	{
		fprintf(fpDump, "\tImageStartOffset = %d\r\n", gped.imageStartOffset);
	}
	else
	{
		fprintf(fpDump, "\tImageStartOffset = (optional and not specified)\r\n");
	}

	if(gped.bImageEndOffset)
	{
		fprintf(fpDump, "\tImageEndOffset = %d\r\n", gped.imageEndOffset);
	}
	else
	{
		fprintf(fpDump, "\tImageEndOffset = (optional and not specified)\r\n");
	}

	if(gped.bFieldDominance)
	{
		fprintf(fpDump, "\tFieldDominance = %d\r\n", gped.fieldDominance);
	}
	else
	{
		fprintf(fpDump, "\tFieldDominance = (optional and not specified)\r\n");
	}

	if(gped.bPictureEssenceCoding)
	{
		fprintf(fpDump, "\tPictureEssenceCoding = ");
		for(i = 0; i < 16; i++)
		{
			fprintf(fpDump, "%02x ", gped.pictureEssenceCoding[i]);
		}
		fprintf(fpDump, "\r\n");
	}
	else
	{
		fprintf(fpDump, "\tPictureEssenceCoding = (encoder optional and not specified)\r\n");
	}
}

void MXFLogger::WriteMXFCDCIPictureEssenceDescriptor(CDCIPictureEssenceDescriptor& cped, FILE* fpDump)
{
	WriteMXFGenericPictureEssenceDescriptor(cped.genericPictureEssenceDescriptor, fpDump);

	fprintf(fpDump, "\tComponentDepth = %d\r\n", cped.componentDepth);
	fprintf(fpDump, "\tHorizontalSubsampling = %d\r\n", cped.horizontalSubsampling);

	if(cped.bVerticalSubsampling)
	{
		fprintf(fpDump, "\tVerticalSubsampling = %d\r\n", cped.verticalSubsampling);
	}
	else
	{
		fprintf(fpDump, "\tVerticalSubsampling = (optional and not specified)\r\n");
	}

	if(cped.bColorSiting)
	{
		char str[1024];
		switch(cped.colorSiting)
		{
			case 0: strcpy(str, "ColorSiting = 0 (coSiting)"); break;
			case 1: strcpy(str, "ColorSiting = 1 (mid-point)"); break;
			case 2: strcpy(str, "ColorSiting = 2 (threeTap)"); break;
			case 3: strcpy(str, "ColorSiting = 3 (QuinCunx)"); break;
			case 4: strcpy(str, "ColorSiting = 4 (Rec601)"); break;
			default: sprintf(str, "ColorSiting = %d (Unknown)", cped.colorSiting); break;
		}
		fprintf(fpDump, "\t%s\r\n", str);
	}
	else
	{
		fprintf(fpDump, "\tColorSiting = (optional and not specified)\r\n");
	}

	if(cped.bReversedByteOrder)
	{
		fprintf(fpDump, "\tReversedByteOrder = %d\r\n", cped.reversedByteOrder);
	}
	else
	{
		fprintf(fpDump, "\tReversedByteOrder = (optional and not specified)\r\n");	
	}

	if(cped.bPaddingBits)
	{
		fprintf(fpDump, "\tPaddingBits = %d\r\n", cped.paddingBits);		
	}
	else
	{
		fprintf(fpDump, "\tPaddingBits = (optional and not specified)\r\n");		
	}

	if(cped.bAlphaSampleDepth)
	{
		fprintf(fpDump, "\tAlphaSampleDepth = %d\r\n", cped.alphaSampleDepth);		
	}
	else
	{
		fprintf(fpDump, "\tAlphaSampleDepth = (optional and not specified)\r\n");		
	}

	if(cped.bBlackRefLevel)
	{
		fprintf(fpDump, "\tBlackRefLevel = %d\r\n", cped.blackRefLevel);		
	}
	else
	{
		fprintf(fpDump, "\tBlackRefLevel = (optional and not specified)\r\n");		
	}

	if(cped.bWhiteRefLevel)
	{
		fprintf(fpDump, "\tWhiteRefLevel = %d\r\n", cped.whiteRefLevel);		
	}
	else
	{
		fprintf(fpDump, "\tWhiteRefLevel = (optional and not specified)\r\n");		
	}

	if(cped.bColorRange)
	{
		fprintf(fpDump, "\tColorRange = %d\r\n", cped.colorRange);		
	}
	else
	{
		fprintf(fpDump, "\tColorRange = (optional and not specified)\r\n");
	}
}

void MXFLogger::WriteMXFGenericSoundEssenceDescriptor(GenericSoundEssenceDescriptor& gsed, FILE* fpDump)
{
	unsigned int i;

	WriteMXFFileDescriptor(gsed.fileDescriptor, fpDump);

	fprintf(fpDump, "\tAudioSamplingRate = %d/%d\r\n", gsed.audioSamplingRate.num, gsed.audioSamplingRate.den);
	
	if(gsed.bLockedUnlocked)
	{
		fprintf(fpDump, "\tLocked/Unlocked = %d\r\n", gsed.lockedUnlocked);
	}
	else
	{
		fprintf(fpDump, "\tLocked/Unlocked = (encoder optional and not specified)\r\n");
	}

	if(gsed.bAudioRefLevel)
	{
		fprintf(fpDump, "\tAudioRefLevel = %d\r\n", gsed.audioRefLevel);
	}
	else
	{
		fprintf(fpDump, "\tAudioRefLevel = (optional and not specified)\r\n");
	}

	if(gsed.bElectroSpatialFormulation)
	{
		char str[1024];
		switch(gsed.electroSpatialFormulation)
		{
			case 0: strcpy(str, "Electro Spatial Formulation = 0 (two-channel mode default)"); break;
			case 1: strcpy(str, "Electro Spatial Formulation = 1 (two-channel mode)"); break;
			case 2: strcpy(str, "Electro Spatial Formulation = 2 (single channel mode)"); break;
			case 3: strcpy(str, "Electro Spatial Formulation = 3 (primary/secondary mode)"); break;
			case 4: strcpy(str, "Electro Spatial Formulation = 4 (stereophonic mode)"); break;
			case 7: strcpy(str, "Electro Spatial Formulation = 7 (single channel, double frequency mode carried on 2 sub-frames)"); break;
			case 8: strcpy(str, "Electro Spatial Formulation = 8 (stereo left channel, double frequency mode carried on 2 sub-frames)"); break;
			case 9: strcpy(str, "Electro Spatial Formulation = 9 (stereo right channel, double frequency mode carried on 2 sub-frames)"); break;
			case 15: strcpy(str, "Electro Spatial Formulation = 15 (multi-channel mode default (>2 channels))"); break;
			default:
				sprintf(str, "Electro Spatial Formulation = %d (undefined)", gsed.electroSpatialFormulation);
		}
		fprintf(fpDump, "\t%s\r\n", str);
	}
	else
	{
		fprintf(fpDump, "\tElectro Spatial Formulation = (optional and not specified)\r\n");
	}

	fprintf(fpDump, "\tChannelCount = %d\r\n", gsed.channelCount);
	fprintf(fpDump, "\tQuantizationBits = %d\r\n", gsed.quantizationBits);

	if(gsed.bDialNorm)
	{
		fprintf(fpDump, "\tDialNorm = %d\r\n", gsed.dialNorm);
	}
	else
	{
		fprintf(fpDump, "\tDialNorm = (optional and not specified)\r\n");
	}

	if(gsed.bSoundEssenceCompression)
	{
		fprintf(fpDump, "\tSoundEssenceCompression = ");
		for(i = 0; i < 16; i++)
		{
			fprintf(fpDump, "%02x ", gsed.soundEssenceCompression[i]);
		}
		fprintf(fpDump, "\r\n");
	}
	else
	{
		fprintf(fpDump, "\tSoundEssenceCompression = (encoder optional and not specified)\r\n");
	}
}

void MXFLogger::WriteMXFSystemMetadataPack(SystemMetadataPack& smp, FILE* fpDump)
{
	unsigned int i;
	fprintf(fpDump, "\tSystemMetadataBitmap = %d\r\n", smp.systemMetadataBitmap);
	fprintf(fpDump, "\tContentPackageRate = %d\r\n", smp.contentPackageRate);
	fprintf(fpDump, "\tContentPackageType = %d\r\n", smp.contentPackageType);
	fprintf(fpDump, "\tChannelHandle = %d\r\n", smp.channelHandle);
	fprintf(fpDump, "\tContinuityCount = %d\r\n", smp.continuityCount);
	
	fprintf(fpDump, "\tUniversalLabel = ");	
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", smp.universalLabel[i]);
	}
	fprintf(fpDump, "\r\n");

	fprintf(fpDump, "\tCreationDateTimeStamp:\r\n");
	fprintf(fpDump, "\t\tType = 0x%02x\r\n", smp.creationDateTimeStamp.type);
	fprintf(fpDump, "\t\tData = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", smp.creationDateTimeStamp.data[i]);
	}
	fprintf(fpDump, "\r\n");
	if(smp.creationDateTimeStamp.type == 0x81)
	{
		// Timecode
		// SMPTE 331
		// Seção 8.2
		SMPTE12MTimecode* tc = (SMPTE12MTimecode*)smp.userDateTimeStamp.data;
		fprintf(fpDump,  "\t\t\t%02d:%02d:%02d:%02d\r\n", 10*tc->hoursTens+tc->hoursUnits, 10*tc->minutesTens+tc->minutesUnits, 10*tc->secondsTens+tc->secondsUnits,		10*tc->frameTens+tc->frameUnits);
	}

	fprintf(fpDump, "\tUserDateTimeStamp:\r\n");
	fprintf(fpDump, "\t\tType = 0x%02x\r\n", smp.userDateTimeStamp.type);
	fprintf(fpDump, "\t\tData = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", smp.userDateTimeStamp.data[i]);
	}
	fprintf(fpDump, "\r\n");
	
	if(smp.userDateTimeStamp.type == 0x81)
	{
		// Timecode
		// SMPTE 331
		// Seção 8.2
		SMPTE12MTimecode* tc = (SMPTE12MTimecode*)smp.userDateTimeStamp.data;
		fprintf(fpDump, "\t\t\tColor Frame Flag (CF) = %d\r\n", tc->cfFlag);
		fprintf(fpDump, "\t\t\tDrop Frame Flag (DF) = %d\r\n", tc->dfFlag);
		fprintf(fpDump, "\t\t\tField Phase (FP) = %d\r\n", tc->fp);
		fprintf(fpDump,  "\t\t\tTimecode = %02d:%02d:%02d:%02d\r\n", 10*tc->hoursTens+tc->hoursUnits, 10*tc->minutesTens+tc->minutesUnits, 10*tc->secondsTens+tc->secondsUnits, 10*tc->frameTens+tc->frameUnits);
		fprintf(fpDump, "\t\t\tBinaryGroup0(NTSC) / BinaryGroup2(PAL) (B0) = %d\r\n", tc->b0);
		fprintf(fpDump, "\t\t\tBinaryGroup2(NTSC) / FieldPhase(PAL) (B1) = %d\r\n", tc->b1);
		fprintf(fpDump, "\t\t\tBinaryGroup1 (B2) = %d\r\n", tc->b2);
		fprintf(fpDump, "\t\t\tBinaryGroupData = 0x%02x 0x%02x 0x%02x 0x%02x\r\n", tc->bg1, tc->bg2, tc->bg3, tc->bg4); 
	}
}

void MXFLogger::WriteMXFMPEG2VideoDescriptor(MPEG2VideoDescriptor& vd, FILE* fp)
{
	WriteMXFCDCIPictureEssenceDescriptor(vd.cdciPictureEssenceDescriptor, fp);
	fprintf(fp, "\t---------------------------\r\n");
	
	if(vd.bSingleSequence)
	{
		fprintf(fp, "\tSingleSequence = %d\r\n", vd.singleSequence);
	}
	else
	{
		fprintf(fp, "\tSingleSequence = (encoder optional and not specified)\r\n");
	}

	if(vd.bConstantBFrames)
	{
		fprintf(fp, "\tConstantBFrames = %d\r\n", vd.constantBFrames);
	}
	else
	{
		fprintf(fp, "\tConstantBFrames = (encoder optional and not specified)\r\n");
	}

	if(vd.bCodedContentType)
	{
		switch(vd.codedContentType)
		{
			case 0:
				fprintf(fp, "\tCodedContentType = 0 (Unknown)\r\n");
			break;
			case 1:
				fprintf(fp, "\tCodedContentType = 1 (Progressive)\r\n");
			break;
			case 2:
				fprintf(fp, "\tCodedContentType = 2 (Interlaced)\r\n");
			break;
			case 3:
				fprintf(fp, "\tCodedContentType = 3 (Mixed)\r\n");
			break;
			default:
				fprintf(fp, "\tCodedContentType = %d (valor não permitido)\r\n", vd.codedContentType);
		}
	}
	else
	{
		fprintf(fp, "\tCodedContentType = (encoder optional and not specified)\r\n");
	}

	if(vd.bLowDelay)
	{
		fprintf(fp, "\tLowDelay = %d\r\n", vd.lowDelay);
	}
	else
	{
		fprintf(fp, "\tLowDelay = (encoder optional and not specified)\r\n");
	}

	if(vd.bClosedGOP)
	{
		fprintf(fp, "\tClosedGOP = %d\r\n", vd.closedGOP);
	}
	else
	{
		fprintf(fp, "\tClosedGOP = (encoder optional and not specified)\r\n");
	}
	if(vd.bIdenticalGOP)
	{
		fprintf(fp, "\tIdenticalGOP = %d\r\n", vd.identicalGOP);
	}
	else
	{
		fprintf(fp, "\tIdenticalGOP = (encoder optional and not specified)\r\n");
	}
	if(vd.bMaxGOP)
	{
		fprintf(fp, "\tMaxGOP = %d\r\n", vd.maxGOP);
	}
	else
	{
		fprintf(fp, "\tMaxGOP = (encoder optional and not specified)\r\n");
	}

	if(vd.bBPictureCount)
	{
		fprintf(fp, "\tBPictureCount = %d\r\n", vd.bPictureCount);
	}
	else
	{
		fprintf(fp, "\tBPictureCount = (encoder optional and not specified)\r\n");
	}

	if(vd.bBitRate)
	{
		fprintf(fp, "\tBitRate = %d\r\n", vd.bitRate);
	}
	else
	{
		fprintf(fp, "\tBitRate = (encoder optional and not specified)\r\n");
	}

	if(vd.bProfileAndLevel)
	{
		fprintf(fp, "\tProfileAndLevel = 0x%02x\r\n", vd.profileAndLevel);
	}
	else
	{
		fprintf(fp, "\tProfileAndLevel = (encoder optional and not specified)\r\n");
	}
}

void MXFLogger::WriteMXFWaveAudioEssenceDescriptor(WaveAudioEssenceDescriptor& wd, FILE* fp)
{
	unsigned int i;

	WriteMXFGenericSoundEssenceDescriptor(wd.genericSoundEssenceDescriptor, fp);

	fprintf(fp, "\t---------------------------\r\n");
	fprintf(fp, "\tBlockAlign = %d\r\n", wd.blockAlign);
	
	if(wd.bSequenceOffset)
	{
		fprintf(fp, "\tSequence Offset = %d\r\n", wd.sequenceOffset);
	}
	else
	{
		fprintf(fp, "\tSequence Offset = (encoder optional and not specified)\r\n");
	}

	fprintf(fp, "\tAvgBps = %d\r\n", wd.avgBps);
	
	if(wd.bChannelAssignment)
	{
		fprintf(fp, "\tChannelAssignment = ");
		for(i = 0; i < 16; i++)
		{
			fprintf(fp, "%02x ", wd.channelAssignment[i]);
		}
		fprintf(fp, "\r\n");
	}
	else
	{
		fprintf(fp, "\tChannelAssignment = (encoder optional and not specified)\r\n");
	}

	if(wd.bPeakEnvelopeVersion)
	{
		fprintf(fp, "\tPeakEnvelopVersion = %d\r\n", wd.peakEnvelopeVersion);
	}
	else
	{
		fprintf(fp, "\tPeakEnvelopVersion = (encoder optional and not specified)\r\n");
	}

	if(wd.bPeakEnvelopeFormat)
	{
		fprintf(fp, "\tPeakEnvelopFormat = %d\r\n", wd.peakEnvelopeFormat);
	}
	else
	{
		fprintf(fp, "\tPeakEnvelopFormat = (encoder optional and not specified)\r\n");
	}

	if(wd.bPointsPerPeakValue)
	{
		fprintf(fp, "\tPointsPerPeakValue = %d\r\n", wd.pointsPerPeakValue);
	}
	else
	{
		fprintf(fp, "\tPointsPerPeakValue = (encoder optional and not specified)\r\n");
	}

	if(wd.bPeakEnvelopeBlockSize)
	{
		fprintf(fp, "\tPeakEnvelopBlockSize = %d\r\n", wd.peakEnvelopeBlockSize);
	}
	else
	{
		fprintf(fp, "\tPeakEnvelopBlockSize = (encoder optional and not specified)\r\n");
	}

	if(wd.bPeakChannels)
	{
		fprintf(fp, "\tPeakChannels = %d\r\n", wd.peakChannels);
	}
	else
	{
		fprintf(fp, "\tPeakChannels = (encoder optional and not specified)\r\n");
	}

	if(wd.bPeakFrames)
	{
		fprintf(fp, "\tPeakFrames = %d\r\n", wd.peakFrames);
	}
	else
	{
		fprintf(fp, "\tPeakFrames = (encoder optional and not specified)\r\n");
	}

	if(wd.bPeakOfPeaksPosition)
	{
		fprintf(fp, "\tPeakOfPeaksPosition = %d\r\n", wd.peakOfPeaksPosition);
	}
	else
	{
		fprintf(fp, "\tPeakOfPeaksPosition = (encoder optional and not specified)\r\n");
	}

	if(wd.bPeakEnvelopeTimestamp)
	{
		fprintf(fp, "\tPeakEnvelopTimestamp = %02d/%02d/%04d - %02d:%02d:%02d-%02dms/4\r\n", wd.peakEnvelopeTimestamp.st.day, wd.peakEnvelopeTimestamp.st.month, wd.peakEnvelopeTimestamp.st.year, wd.peakEnvelopeTimestamp.st.hour, wd.peakEnvelopeTimestamp.st.minute, wd.peakEnvelopeTimestamp.st.second, wd.peakEnvelopeTimestamp.st.quartermsec);
	}
	else
	{
		fprintf(fp, "\tPeakEnvelopTimestamp = (encoder optional and not specified)\r\n");
	}

	if(wd.bPeakEnvelopeData)
	{
		fprintf(fp, "Falta fazer o dump!!!\r\n");
	}
	else
	{
		fprintf(fp, "\tPeakEnvelopeData = (encoder optional and not specified)\r\n");
	}
}

void MXFLogger::WriteMXFAES3AudioEssenceDescriptor(AES3AudioEssenceDescriptor& ad, FILE* fp)
{
	unsigned int i, j;
	WriteMXFWaveAudioEssenceDescriptor(ad.waveAudioEssenceDescriptor, fp);

	fprintf(fp, "\t---------------------------\r\n");

	if(ad.bEmphasis)
	{
		fprintf(fp, "\tEmphasis = %d\r\n", ad.emphasis);
	}
	else
	{
		fprintf(fp, "\tEmphasis = (encoder optional and not specified)\r\n");
	}

	if(ad.bBlockStartOffset)
	{
		fprintf(fp, "\tBlockStartOffset = %d\r\n", ad.blockStartOffset);
	}
	else
	{
		fprintf(fp, "\tBlockStartOffset = (encoder optional and not specified)\r\n");
	}

	if(ad.bAuxBitsMode)
	{
		fprintf(fp, "\tAuxBitsMode = %d\r\n", ad.auxBitsMode);
	}
	else
	{
		fprintf(fp, "\tAuxBitsMode = (encoder optional and not specified)\r\n");
	}

	if(ad.bChannelStatusMode)
	{
		fprintf(fp, "\tChannelStatusMode:\r\n");
		fprintf(fp, "\t\tNum = %d\r\n", ad.channelStatusMode.num);
		fprintf(fp, "\t\tSize = %d\r\n", ad.channelStatusMode.size);

		for(i = 0; i < ad.channelStatusMode.num; i++)
		{
			int v = ad.channelStatusMode.pElements[i];
			fprintf(fp, "\t\t%d => %d\r\n", i, v);
		}
	}
	else
	{
		fprintf(fp, "\tChannelStatusMode = (encoder optional and not specified)\r\n");
	}

	if(ad.bFixedChannelStatusData)
	{
		fprintf(fp, "\tFixedChannelStatusData:\r\n");
		fprintf(fp, "\t\tNum = %d\r\n", ad.fixedChannelStatusData.num);
		fprintf(fp, "\t\tSize = %d\r\n", ad.fixedChannelStatusData.size);

		for(i = 0; i < ad.fixedChannelStatusData.num; i++)
		{
			fprintf(fp, "\t\t");
			for(j = 0; j < ad.fixedChannelStatusData.size; j++)
			{
				fprintf(fp, "%02x ", ad.fixedChannelStatusData.ppItem[i][j]);
			}
			fprintf(fp, "\r\n");
		}

	}
	else
	{
		fprintf(fp, "\tFixedChannelStatusData = (encoder optional and not specified)\r\n");
	}

	if(ad.bUserDataMode)
	{
		fprintf(fp, "\tUserDataMode:\r\n");
		fprintf(fp, "\t\tNum = %d\r\n", ad.userDataMode.num);
		fprintf(fp, "\t\tSize = %d\r\n", ad.userDataMode.size);

		for(i = 0; i < ad.userDataMode.num; i++)
		{
			int v = ad.userDataMode.pElements[i];
			fprintf(fp, "\t\t%d => %d\r\n", i, v);
		}

	}
	else
	{
		fprintf(fp, "\tUserDataMode = (encoder optional and not specified)\r\n");
	}

	if(ad.bFixedUserData)
	{
		fprintf(fp, "\tFixedUserData:\r\n");
		fprintf(fp, "\t\tNum = %d\r\n", ad.fixedUserData.num);
		fprintf(fp, "\t\tSize = %d\r\n", ad.fixedUserData.size);

		for(i = 0; i < ad.fixedUserData.num; i++)
		{
			fprintf(fp, "\t\t");
			for(j = 0; j < ad.fixedUserData.size; j++)
			{
				fprintf(fp, "%02x ", ad.fixedUserData.ppItem[i][j]);
			}
			fprintf(fp, "\r\n");
		}

	}
	else
	{
		fprintf(fp, "\tFixedUserData = (encoder optional and not specified)\r\n");
	}
}

void MXFLogger::WriteMXFRandomIndexPack(RandomIndexPack& rip, FILE* fp)
{
	unsigned int i;

	fprintf(fp, "\tNum = %d\r\n", rip.numElements);
	fprintf(fp, "\t-------------------------\r\n");
	for(i = 0; i < rip.numElements; i++)
	{
		fprintf(fp, "\tBodySID = %d\r\n", rip.pElements[i].bodySID);
		fprintf(fp, "\tByteOffset = %I64d\r\n", rip.pElements[i].byteOffset);
		fprintf(fp, "\t-------------------------\r\n");
	}

	fprintf(fp, "\tLength = %d\r\n", rip.overallLength);
}

void MXFLogger::WriteMXFDMSegment(DMSegment& dms, FILE* fpDump)
{
	unsigned int i, j;

	fprintf(fpDump, "\tInstanceUID = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", dms.instanceUID[i]);
	}
	fprintf(fpDump, "\r\n");
	
	if(dms.bGenerationUID)
	{
		fprintf(fpDump, "\tGenerationUID = ");
		for(i = 0; i < 16; i++)
		{
			fprintf(fpDump, "%02x ", dms.generationUID[i]);
		}
		fprintf(fpDump, "\r\n");
	}
	else
	{
		fprintf(fpDump, "\tGenerationUID = (optional and not specified)\r\n");
	}

	fprintf(fpDump, "\tData Definition = ");
	for(i = 0; i < 16; i++)
	{
		fprintf(fpDump, "%02x ", dms.dataDefinition[i]);
	}
	fprintf(fpDump, "\r\n");

	if(dms.bEventStartPosition)
	{
		fprintf(fpDump, "\tEvent Start Position = %I64d\r\n", dms.eventStartPosition);
	}
	else
	{
		fprintf(fpDump, "\tEvent Start Position = (optional and not specified)\r\n");
	}

	if(dms.bDuration)
	{
		fprintf(fpDump, "\tDuration = %I64d\r\n", dms.duration);
	}
	else
	{
		fprintf(fpDump, "\tDuration = (optional and not specified)\r\n");
	}

	if(dms.bEventComment)
	{
		fprintf(fpDump, "\tEvent Comment = %S\r\n", dms.eventComment);
	}
	else
	{
		fprintf(fpDump, "\tEvent Comment = (optional and not specified)\r\n");
	}

	if(dms.bTrackID)
	{
		fprintf(fpDump, "\tTrackIDs:\r\n");
		fprintf(fpDump, "\t\tNum = %d\r\n", dms.trackID.num);
		fprintf(fpDump, "\t\tSize = %d\r\n", dms.trackID.size);

		for(i = 0; i < dms.trackID.num; i++)
		{
			fprintf(fpDump, "\t\t");
			for(j = 0; j < dms.trackID.size; j++)
			{
				fprintf(fpDump, "%02x ", dms.trackID.ppItem[i][j]);
			}
			fprintf(fpDump, "\r\n");
		}
	}
	else
	{
		fprintf(fpDump, "\tTrackIDs = (optional and not specified)\r\n");
	}

	if(dms.bDMFramework)
	{
		fprintf(fpDump, "\tDMFramework = ");
		for(i = 0; i < 16; i++)
		{
			fprintf(fpDump, "%02x ", dms.DMFramework[i]);
		}
		fprintf(fpDump, "\r\n");
	}
	else
	{
		fprintf(fpDump, "\tDMFramework = (optional and not specified)\r\n");
	}
}
