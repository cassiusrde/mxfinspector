// MXFInspectorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MXFInspector.h"
#include "MXFInspectorDlg.h"

#include "GoToDlg.h"
#include "HexDumpDlg.h"
#include "MXFLog.h"

#include "MXFUtils.h"

#include "AESDecoder.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BOOL IsPack(CString node)
{
	if( node == L"HEADER" ||
		node == L"BODY" ||
		node == L"PRIMER" ||
		node == L"FOOTER" ||
		//node == L"FILL" ||
		//node == L"KLV FILL" ||
		node == L"INDEX TABLE" ||
		node == L"SYSTEM METADATA PACK" ||
		node == L"PACKAGE METADATA SET" ||
		node == L"RANDOM INDEX PACK" )
		return TRUE;

	if( node == L"UNKNOWN PACK" )		
		return TRUE;

	return FALSE;
}

BOOL IsPicture(CString node)
{
	if( node == L"PICTURE" )
		return TRUE;
	
	return FALSE;
}

BOOL IsSound(CString node)
{
	if( node == L"SOUND" )
		return TRUE;
	
	return FALSE;
}

BOOL IsMetadata(CString node)
{
	if( node == L"METADATA" )
		return TRUE;
	
	return FALSE;
}

BOOL IsIndexTable(CString node)
{
	if( node == L"INDEX TABLE" )
		return TRUE;
	
	return FALSE;
}

// CMXFInspectorDlg dialog
CMXFInspectorDlg::CMXFInspectorDlg(CWnd* pParent /*=NULL*/)
: CDialog(CMXFInspectorDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_threadHnd = 0;
	m_audioType = PCM_AUDIO;
}

void CMXFInspectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, m_treeCtrl);
	DDX_Control(pDX, IDC_LIST1, m_listCtrl);
	DDX_Control(pDX, IDC_EDIT1, m_editHex);	
	DDX_Control(pDX, IDC_PROGRESS1, m_progressCtrl);
}

BEGIN_MESSAGE_MAP(CMXFInspectorDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DROPFILES()
	ON_WM_SIZE()	
	ON_NOTIFY(NM_CLICK, IDC_LIST1, OnNmItemclickList)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnNMDblclkList)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE1, OnNMDblclkTree)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_EXPORTDUMP, &CMXFInspectorDlg::OnFileExportdump)
	ON_COMMAND(ID_FILE_EXPORTVIDEO, &CMXFInspectorDlg::OnFileExportvideo)
	ON_COMMAND(ID_FILE_EXPORTAUDIO, &CMXFInspectorDlg::OnFileExportaudio)
	ON_COMMAND(ID_FILTER_PACK, &CMXFInspectorDlg::OnFilterPack)
	ON_COMMAND(ID_FILTER_PICTURE, &CMXFInspectorDlg::OnFilterPicture)
	ON_COMMAND(ID_FILTER_SOUND, &CMXFInspectorDlg::OnFilterSound)
	ON_COMMAND(ID_FILTER_METADATA, &CMXFInspectorDlg::OnFilterMetadata)
	ON_COMMAND(ID_FILTER_INDEXTABLE, &CMXFInspectorDlg::OnFilterIndextable)
	ON_COMMAND(ID_FILTER_SHOWALL, &CMXFInspectorDlg::OnFilterShowAll)
	ON_COMMAND(ID_GOTO, &CMXFInspectorDlg::OnGoto)
	ON_COMMAND(ID_SAVE, &CMXFInspectorDlg::OnSave)	
END_MESSAGE_MAP()


// CMXFInspectorDlg message handlers

BOOL CMXFInspectorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	DragAcceptFiles(TRUE);

	GetWindowText(m_title);

	CRect rc;
	CWnd* pWnd;

	pWnd = GetDlgItem(IDC_SPLITTER);
	pWnd->GetWindowRect(rc);
	ScreenToClient(rc);
	m_wndSplitter.Create(WS_CHILD | WS_VISIBLE, rc, this, IDC_SPLITTER);
	m_wndSplitter.SetRange(400, 400, -1);
	
	m_listCtrl.InsertColumn(0, L"Offset", LVCFMT_RIGHT, 160);	
	m_listCtrl.InsertColumn(1, L"Info", LVCFMT_LEFT, 320);
	m_listCtrl.InsertColumn(2, L"Length" ,LVCFMT_RIGHT, 60);		
	m_listCtrl.InsertColumn(3, L"Key/Value", LVCFMT_LEFT, 420);
	m_listCtrl.SetExtendedStyle(m_listCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_SINGLEROW);

	m_progressCtrl.SetRange(0, 100);

	m_editHex.SetNotUsedCol(RGB(255,255,255));
	m_editHex.SetAddressSize(8, false);
	m_editHex.SetAdrCol(RGB(255,255,255), RGB(125,125,125));
	m_editHex.SetShowAddress(true, false);
	m_editHex.SetBytesPerRow(16, false, false);
	m_editHex.SetHexCol(RGB(255,255,255), RGB(0,0,0));	
	m_editHex.SetReadonly(true);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMXFInspectorDlg::OnDestroy()
{
	if( m_threadHnd != 0 )
	{
		WaitForSingleObject(m_threadHnd, INFINITE);
		CloseHandle(m_threadHnd);
		m_threadHnd = 0;	
	}

	CDialog::OnDestroy();
}


// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMXFInspectorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

LRESULT CMXFInspectorDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (message == WM_NOTIFY)
	{
		if (wParam == IDC_SPLITTER)
		{	
			SPC_NMHDR* pHdr = (SPC_NMHDR*) lParam;
			DoResize(pHdr->delta);
		}		
	}
	
	return CDialog::DefWindowProc(message, wParam, lParam);
}

void CMXFInspectorDlg::DoResize(int delta)
{
	CSplitterControl::ChangeWidth(&m_listCtrl, delta);
	CSplitterControl::ChangeWidth(&m_progressCtrl, delta);
	CSplitterControl::ChangeWidth(&m_treeCtrl, -delta, CW_RIGHTALIGN);
	CSplitterControl::ChangeWidth(&m_editHex, -delta, CW_RIGHTALIGN);	
	Invalidate();
	UpdateWindow();
}

void CMXFInspectorDlg::OnSize(UINT nType, int cx, int cy) 
{ 	
	if( nType != SIZE_MINIMIZED )
	{	
		if( m_listCtrl && m_treeCtrl && m_editHex )
		{
			CRect listRect, treeRect, editRect, progressRect, splitterRect;	

			double xscale = (double)cx / m_lastSize.Width();

			double yscale = (double)cy / m_lastSize.Height();

			m_listCtrl.GetWindowRect(&listRect);		
			ScreenToClient(&listRect);

			m_treeCtrl.GetWindowRect(&treeRect);	
			ScreenToClient(&treeRect);				

			m_editHex.GetWindowRect(&editRect);	
			ScreenToClient(&editRect);		

			m_progressCtrl.GetWindowRect(&progressRect);	
			ScreenToClient(&progressRect);	

			m_wndSplitter.GetWindowRect(&splitterRect);	
			ScreenToClient(&splitterRect);	

			int x, y, h, w;

			x = 5;
			y = 10;
			w = (int)(listRect.Width()*xscale);
			h = (cy - progressRect.Height() - 10 - 5 - 5);
			m_listCtrl.SetWindowPos(&m_listCtrl, x, y, w, h, SWP_NOZORDER |SWP_SHOWWINDOW);
			m_listCtrl.RedrawWindow();

			x = 5 + (int)(listRect.Width()*xscale);	
			y = 10;
			w = 5;
			h = (cy - 10 - 5);
			m_wndSplitter.SetWindowPos(&m_wndSplitter, x, y, w, h, SWP_NOZORDER |SWP_SHOWWINDOW);
			m_wndSplitter.RedrawWindow();

			x = 5 + (int)(listRect.Width()*xscale) + 5;	
			y = 10;
			w = (cx - x - 5);
			h = (int)(treeRect.Height()*yscale);
			m_treeCtrl.SetWindowPos(&m_treeCtrl, x, y, w, h, SWP_NOZORDER |SWP_SHOWWINDOW);
			m_treeCtrl.RedrawWindow();
			
			//x = 5 + 5 + (int)(listRect.Width()*xscale);	
			y = 10 + h + 5;
			w = (cx - x - 5);
			h = (cy - y - 5);
			m_editHex.SetWindowPos(&m_editHex, x, y, w, h, SWP_NOZORDER |SWP_SHOWWINDOW);	
			m_editHex.RedrawWindow();
			
			x = 5;	
			y = cy - progressRect.Height() - 5;
			w = (int)(progressRect.Width()*xscale);
			h = progressRect.Height();
			
			m_progressCtrl.SetWindowPos(&m_progressCtrl, x, y, w, h, SWP_NOZORDER |SWP_SHOWWINDOW);
			m_progressCtrl.RedrawWindow();
		}

		m_lastSize = CRect(0, 0, cx, cy);
	}

	CDialog::OnSize(nType, cx, cy);
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMXFInspectorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


afx_msg void CMXFInspectorDlg::OnDropFiles(HDROP hDropInfo)
{
	UINT nb_files = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);

	if (nb_files > 0)
	{
		// start thread
		if( m_threadHnd != 0 )
		{
			if (WaitForSingleObject(m_threadHnd, 1000) == WAIT_TIMEOUT )
			{
				DragFinish(hDropInfo);
				return;
			}
			CloseHandle(m_threadHnd);
			m_threadHnd = 0;	
		}

		m_list.clear();
		m_listCtrl.DeleteAllItems();
		m_treeCtrl.DeleteAllItems();

		UINT buffer_size = DragQueryFile(hDropInfo, 0, NULL, 0) + 1;
		wchar_t *buffer = new wchar_t [buffer_size];
		DragQueryFile(hDropInfo, 0, buffer, buffer_size);
		
		m_title = L"MXFInpesctor - " + CString(buffer);

		SetWindowText(m_title);

		m_fnameMXF = CString(buffer);		
		delete [] buffer;

		m_threadHnd = ::CreateThread(0, 0, LPTHREAD_START_ROUTINE(threadProc), this, 0, 0);		
	}

	DragFinish(hDropInfo);
}

unsigned long CMXFInspectorDlg::threadProc(CMXFInspectorDlg* pDlg)
{	
	if( !pDlg->m_fnameMXF.IsEmpty() )
		pDlg->DumpMXF(pDlg->m_fnameMXF);

	return 0;
}

BOOL CMXFInspectorDlg::DumpMXF(CString fnameMXF)
{	
	unsigned char keyValue[16];
	unsigned char ch;
	DWORD size, szSize, nRead;	
	BOOL bSeek, bCont;
	HANDLE hMXF = 0;;
	PrimerPack primerPack;

	unsigned __int64 footer_partition = 0;
	unsigned __int64 header_byte_count = 0;	
	unsigned __int64 first_body = 0;


	hMXF = CreateFile(fnameMXF, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hMXF == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(L"Erro ao abrir arquivo MXF\n");		
		return FALSE;
	}

	bCont = TRUE;
	unsigned __int64 filePos, fileSize;

	fileSize = GetFileSize(hMXF);
	
	while(bCont)
	{
		filePos = GetFilePosition(hMXF);
		BOOL r = ReadFile(hMXF, keyValue, 16, &nRead, NULL);
		if(nRead < 16)
		{
			DWORD d = GetLastError();
			break; // EOF
		}


		ReadFile(hMXF, &ch, 1, &nRead, NULL);

		if( ch & 0x80 )
		{
			szSize = ch&0x0f;
			size = (DWORD)GetNumber(hMXF, szSize);
		}
		else
		{
			size = ch;
		}

		bSeek = TRUE;		

		if(!memcmp(keyValue, gcpictureItemKey, sizeof(gcpictureItemKey)) )
		{
			switch(keyValue[14])
			{
				case 0x05: InsertEntry(filePos, L"PICTURE", keyValue, size, L"<MPEG2>"); break;
				default: InsertEntry(filePos, L"PICTURE", keyValue, size, L"<UNKNOWN>"); break;
			}

			bSeek = TRUE;
		}
		else
		if(!memcmp(keyValue, cppictureItemKey, sizeof(cppictureItemKey)) )
		{
			switch(keyValue[14])
			{
				case 0x01: InsertEntry(filePos, L"PICTURE", keyValue, size, L"<IMX>"); break;
				default: InsertEntry(filePos, L"PICTURE", keyValue, size, L"<UNKNOWN>"); break;
			}

			bSeek = TRUE;
		}
		else
		if(!memcmp(keyValue, gccompoundItemKey, sizeof(gccompoundItemKey)) )
		{
			switch(keyValue[14])
			{
				case 0x01: InsertEntry(filePos, L"PICTURE", keyValue, size, L"<DV>"); break;
				default: InsertEntry(filePos, L"PICTURE", keyValue, size, L"<UNKNOWN>"); break;
			}

			bSeek = TRUE;
		}
		else
		if(!memcmp(keyValue, gcsoundItemKey, sizeof(gcsoundItemKey)) )
		{
			switch(keyValue[14])
			{
				case 0x01: InsertEntry(filePos, L"SOUND", keyValue, size, L"<PCM S16LE>"); break;
				case 0x03: InsertEntry(filePos, L"SOUND", keyValue, size, L"<AES>"); break;
				case 0x05: InsertEntry(filePos, L"SOUND", keyValue, size, L"<MPEG>"); break;
				default: InsertEntry(filePos, L"SOUND", keyValue, size, L"<UNKNOWN>"); break;
			}

			bSeek = TRUE;
		}
		else
		if(!memcmp(keyValue, cpsoundItemKey, sizeof(cpsoundItemKey)) )
		{
			switch(keyValue[14])
			{
				case 0x010: InsertEntry(filePos, L"SOUND", keyValue, size, L"<AES3 8CH>"); break;
				default: InsertEntry(filePos, L"SOUND", keyValue, size, L"<UNKNOWN>"); break;
			}
			bSeek = TRUE;
		}
		else
		if(!memcmp(keyValue, gcdataItemKey, sizeof(gcdataItemKey)) )
		{
			InsertEntry(filePos, L"DATA", keyValue, size, L"<GENERIC CONTAINER DATA>");
			bSeek = TRUE;
		}
		else
		if(!memcmp(keyValue, cpdataItemKey, sizeof(cpdataItemKey)) )
		{
			InsertEntry(filePos, L"DATA", keyValue, size, L"<CONTENT PACKAGE DATA>");
			bSeek = TRUE;
		}
		else
		if(!memcmp(keyValue, headerKey, sizeof(headerKey)))
		{
			int status = keyValue[14];
			int reserved = keyValue[15];
			ASSERT(reserved == 0x00);

			CString str;
			str.Format(L"HEADER status = %x", status);

			InsertEntry(filePos, L"HEADER", keyValue, size, str);

			PartitionPack pack;
			pack.Read(hMXF, size);
			bSeek = FALSE;

			footer_partition = pack.footerPartition;
			header_byte_count = pack.headerByteCount;
		}
		else
		if(!memcmp(keyValue, bodyKey, sizeof(bodyKey)))
		{
			int status = keyValue[14];
			int reserved = keyValue[15];
			ASSERT(reserved == 0x00);

			CString str;
			str.Format(L"BODY status = %x", status);

			InsertEntry(filePos, L"BODY", keyValue, size, str);

			PartitionPack pack;
			pack.Read(hMXF, size);
			bSeek = FALSE;	

			if( header_byte_count )
			{
				//ASSERT( header_byte_count == (filePos - pack.kagSize) );
				header_byte_count = 0;
			}				

			/*
			if( first_body )
				ASSERT( (pack.bodyOffset + pack.kagSize) == (filePos - first_body) );
			else
				first_body = filePos;
			*/			
		}
		else
		if(!memcmp(keyValue, primerPackKey, sizeof(primerPackKey)))
		{
			InsertEntry(filePos, L"PRIMER", keyValue, size, L"PRIMER PACK");

			m_primerPack.Read(hMXF);
			bSeek = FALSE;	
		}
		else
		if(!memcmp(keyValue, footerKey, sizeof(footerKey)))
		{
			int status = keyValue[14];
			int reserved = keyValue[15];
			ASSERT(reserved == 0x00);

			CString str;
			str.Format(L"FOOTER status = %x", status);
			InsertEntry(filePos, L"FOOTER", keyValue, size, str);

			ASSERT( footer_partition == filePos );
		}
		else
		if(!memcmp(keyValue, fillKey, sizeof(fillKey)))
		{
			InsertEntry(filePos, L"FILL", keyValue, size, L"FILL");
		}
		else
		if(!memcmp(keyValue, klvFillDataKey, sizeof(klvFillDataKey)))
		{
			InsertEntry(filePos, L"KLV FILL", keyValue, size, L"KLV FILL DATA");
		}
		else
		if(!memcmp(keyValue, metadataKey, sizeof(metadataKey)))
		{
			switch(keyValue[13]*256+keyValue[14])
			{
				case 0x012f: InsertEntry(filePos, L"METADATA", keyValue, size, L"<Preface>"); break;
				case 0x0130: InsertEntry(filePos, L"METADATA", keyValue, size, L"<Identification>"); break;
				case 0x0118: InsertEntry(filePos, L"METADATA", keyValue, size, L"<Content Storage>"); break;
				case 0x0123: InsertEntry(filePos, L"METADATA", keyValue, size, L"<Essence Container Data>"); break;
				case 0x0136: InsertEntry(filePos, L"METADATA", keyValue, size, L"<Material Package>"); break;	
				case 0x0137: InsertEntry(filePos, L"METADATA", keyValue, size, L"<Source Package (File / Physical)>"); break;
				case 0x013b: InsertEntry(filePos, L"METADATA", keyValue, size, L"<Track (Timeline)>"); break;
				case 0x0139: InsertEntry(filePos, L"METADATA", keyValue, size, L"<Event Track>"); break;	
				case 0x013a: InsertEntry(filePos, L"METADATA", keyValue, size, L"<Static Track>"); break;
				case 0x010f: InsertEntry(filePos, L"METADATA", keyValue, size, L"<Sequence (all cases)>"); break;
				case 0x0111: InsertEntry(filePos, L"METADATA", keyValue, size, L"<SourceClip (Picture, Sound, Data)>"); break;
				case 0x0114: InsertEntry(filePos, L"METADATA", keyValue, size, L"<Timecode Component>"); break;
				case 0x0141: InsertEntry(filePos, L"METADATA", keyValue, size, L"<DM Segment>"); break;
				case 0x0145: InsertEntry(filePos, L"METADATA", keyValue, size, L"<DM SourceClip>"); break;
				case 0x0125: InsertEntry(filePos, L"METADATA", keyValue, size, L"<File Descriptor>"); break;
				case 0x0127: InsertEntry(filePos, L"METADATA", keyValue, size, L"<Generic Picture Essence Descriptor>"); break;
				case 0x0128: InsertEntry(filePos, L"METADATA", keyValue, size, L"<CDCI Essence Descriptor>"); break;
				case 0x0129: InsertEntry(filePos, L"METADATA", keyValue, size, L"<RGBA Essence Descriptor>"); break;
				case 0x0142: InsertEntry(filePos, L"METADATA", keyValue, size, L"<Generic Sound Essence Descriptor>"); break;
				case 0x0143: InsertEntry(filePos, L"METADATA", keyValue, size, L"<Generic Data Essence Descriptor>"); break;
				case 0x0144: InsertEntry(filePos, L"METADATA", keyValue, size, L"<Multiple Descriptor>"); break;
				case 0x0132: InsertEntry(filePos, L"METADATA", keyValue, size, L"<Network Locator>"); break;
				case 0x0133: InsertEntry(filePos, L"METADATA", keyValue, size, L"<Text Locator>"); break;
				case 0x0151: InsertEntry(filePos, L"METADATA", keyValue, size, L"<MPEG2VideoDescriptor>"); break;					
				case 0x0147: InsertEntry(filePos, L"METADATA", keyValue, size, L"<AES3AudioEssenceDescriptor>"); break;
				case 0x0148: InsertEntry(filePos, L"METADATA", keyValue, size, L"<WaveAudioEssenceDescriptor>"); break;
				default: InsertEntry(filePos, L"METADATA", keyValue, size, L"<UNKNOWN>"); break;				
			}			
		}
		else
		if(!memcmp(keyValue, indexTableKey, sizeof(indexTableKey)))
		{
			InsertEntry(filePos, L"INDEX TABLE", keyValue, size, L"INDEX TABLE");			
		}
		else
		if(!memcmp(keyValue, systemMetadataPackKey, sizeof(systemMetadataPackKey)))
		{
			InsertEntry(filePos, L"SYSTEM METADATA PACK", keyValue, size, L"SYSTEM METADATA PACK");
		}
		else
		if(!memcmp(keyValue, packageMetadataSetKey, sizeof(packageMetadataSetKey)))
		{
			int blockCount = keyValue[15];
			CString str;
			str.Format(L"[PACKAGE METADATA SET] => MetadataBlockCount = %d", blockCount);
			InsertEntry(filePos, L"PACKAGE METADATA SET", keyValue, size, str);
		}
		else
		if(!memcmp(keyValue, xmlDocumentTextKey, sizeof(xmlDocumentTextKey)))
		{
			InsertEntry(filePos, L"XMLDocumentText", keyValue, size, L"XMLDocumentText");
		}
		else
		if(!memcmp(keyValue, randomIndexPackKey, sizeof(randomIndexPackKey)))
		{			
			InsertEntry(filePos, L"RANDOM INDEX PACK", keyValue, size, L"RANDOM INDEX PACK");
		}
		else
		if(!memcmp(keyValue, darkdataKey, sizeof(darkdataKey)))
		{			
			InsertEntry(filePos, L"DARK METADATA", keyValue, size, L"DARK METADATA");
		}
		else
		if(!memcmp(keyValue, dms1ItemKey, sizeof(dms1ItemKey)))
		{	
			short key  = keyValue[13]*256+keyValue[14];
			switch(key)
			{
				case 0x0101: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Production Framework>"); break;
				case 0x0102: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Clip Framework>"); break;
				case 0x0103: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Scene Framework>"); break;
				case 0x1001: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Titles>"); break;
				case 0x1101: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Identification>"); break;
				case 0x1201: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Group Relationship>"); break;
				case 0x1301: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Branding"); break;
				case 0x1401: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Event>"); break;
				case 0x1402: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Publication>"); break;
				case 0x1501: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Award"); break;
				case 0x1601: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Caption Description>"); break;
				case 0x1701: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Annotation>"); break;
				case 0x1702: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Setting Period>"); break;
				case 0x1703: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Scripting>"); break;
				case 0x1704: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Classification>"); break;
				case 0x1705: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Shot>"); break;
				case 0x1706: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Key Point>"); break;
				case 0x1801: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Participant>"); break;
				case 0x1A02: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Person>"); break;
				case 0x1A03: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Organization>"); break;
				case 0x1A04: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Location>"); break;
				case 0x1B01: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Address>"); break;
				case 0x1B02: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Communucations>"); break;
				case 0x1C01: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Contract>"); break;
				case 0x1C02: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Rights>"); break;
				case 0x1D01: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Picture Format>"); break;
				case 0x1E01: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Device Parameters>"); break;
				case 0x1F01: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Name-Value>"); break;
				case 0x2001: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Processing>"); break;
				case 0x2002: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Project>"); break;
				case 0x1901: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Contacts List>"); break;
				case 0x1708: InsertEntry(filePos, L"DMS-1", keyValue, size, L"<Cue Words>"); break;
			}		
		}
		else
		{
			InsertEntry(filePos, L"UNKNOWN PACK", keyValue, size, L"UNKNOWN PACK");
		}

		if(bSeek)
		{
			LARGE_INTEGER pos = {0};
			DWORD ret;

			pos.QuadPart = size;
			ret = SetFilePointer(hMXF, pos.LowPart, &pos.HighPart, FILE_CURRENT);
			if(ret == INVALID_SET_FILE_POINTER && GetLastError() != ERROR_SUCCESS)
				break;			
		}

		unsigned __int64 pos = (100*filePos) / fileSize;
		m_progressCtrl.SetPos((int)pos);
	}

	m_progressCtrl.SetPos(100);

	if( hMXF )
		CloseHandle(hMXF);

	return TRUE;
}

void CMXFInspectorDlg::InsertEntry(unsigned __int64 offset, CString node, unsigned char key[16], DWORD length, CString info, bool save)
{
	CString str;

	str.Format(L"<%016I64x>", offset);

	int nIndex = m_listCtrl.GetItemCount();

	// Offset
	m_listCtrl.InsertItem(nIndex, str);

	// Info
	m_listCtrl.SetItemText(nIndex, 1, info);	

	str.Format(L"%d", length);
	// Length
	m_listCtrl.SetItemText(nIndex, 2, str);	

	str  = L"{ ";
	for(int i = 0; i < 16; i++)
	{
		CString temp;
		temp.Format(L"%02x ", key[i]);
		str += temp;
	}
	str  += L"}";

	// Key
	m_listCtrl.SetItemText(nIndex, 3, str);	

	if( save )
		m_list.push_back(MyItemList(offset, node, key, length, info));
}

void CMXFInspectorDlg::OnNmItemclickList(NMHDR* pNMHDR, LRESULT* pResult)
{
	POSITION pos = m_listCtrl.GetFirstSelectedItemPosition();

	if(pos != NULL)	
	{
		int itemSelect = m_listCtrl.GetNextSelectedItem(pos);
		CString str = m_listCtrl.GetItemText(itemSelect, 0);
		
		const wchar_t temp[20] = {0};		
		swscanf(str.GetString(), L"<%s>", temp);
		unsigned __int64 offset = _wcstoui64(temp, NULL, 16);

		//unsigned __int64 offset = m_list[itemSelect].offset;

		DumpEntry(offset);
		ShowEntry(offset);
	}

	*pResult = 0;	
}

void CMXFInspectorDlg::OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	POSITION pos = m_listCtrl.GetFirstSelectedItemPosition();

	if(pos != NULL)	
	{
		int itemSelect = m_listCtrl.GetNextSelectedItem(pos);
		CString str = m_listCtrl.GetItemText(itemSelect, 0);
		
		const wchar_t temp[20] = {0};		
		swscanf(str.GetString(), L"<%s>", temp);
		unsigned __int64 offset = _wcstoui64(temp, NULL, 16);
		ShowEntry(offset);
	}

	*pResult = 0;	
}


void CMXFInspectorDlg::OnNMDblclkTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	/*NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	TVITEM tvitem = (TVITEM)pNMTreeView->itemNew;

	if( tvitem.hItem )
	{
		m_treeCtrl.Expand(tvitem.hItem, TVE_TOGGLE);

		BOOL ret = m_treeCtrl.ItemHasChildren(tvitem.hItem);

		HTREEITEM hNextItem;
	    HTREEITEM hChildItem = m_treeCtrl.GetChildItem(tvitem.hItem);

		while (hChildItem != NULL)
		{
			hNextItem = m_treeCtrl.GetNextItem(hChildItem, TVGN_NEXT);
			m_treeCtrl.Expand(hChildItem, TVE_TOGGLE);
			hChildItem = hNextItem;
		}
	}*/


	//unsigned __int64 offset = 0;
	//HTREEITEM selected = m_treeCtrl.GetSelectedItem();

	//if(selected != NULL)	
	//{		
	//	m_treeCtrl.Expand(selected, TVE_TOGGLE);
	//	//offset = (unsigned __int64)m_treeCtrl.GetItemData(selected);
	//}

	*pResult = 0;	
}

BOOL CMXFInspectorDlg::ShowEntry(unsigned __int64 offset)
{
	unsigned char keyValue[16];
	unsigned char ch;
	DWORD size, szSize, nRead;	
	HANDLE hMXF;

	hMXF = CreateFile(m_fnameMXF, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hMXF == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(L"Erro ao abrir arquivo MXF\n");
		return FALSE;
	}

	unsigned __int64 filePos;

	// seek
	LARGE_INTEGER pos = {0};
	DWORD ret;

	pos.QuadPart = offset;
	ret = SetFilePointer(hMXF, pos.LowPart, &pos.HighPart, FILE_CURRENT);
	if(ret == INVALID_SET_FILE_POINTER && GetLastError() != ERROR_SUCCESS)
	{		
		return FALSE;
	}

	filePos = GetFilePosition(hMXF);

	ReadFile(hMXF, keyValue, 16, &nRead, NULL);

	if(nRead < 16)
	{		
		return FALSE; // EOF
	}

	ASSERT(filePos == offset);

	ReadFile(hMXF, &ch, 1, &nRead, NULL);
	if( ch & 0x80 )
	{
		szSize = ch&0x0f;
		size = (DWORD)GetNumber(hMXF, szSize);
	}
	else
	{
		size = ch;
	}
	
	LPBYTE pBuffer = new BYTE[size];
	ReadFile(hMXF, pBuffer, size, &nRead, NULL);

	//CHexDumpDlg dlg;
	//dlg.SetMemory(pBuffer, size);
	
	if( size )
		m_editHex.SetData(pBuffer, size);

	delete []pBuffer;

	//dlg.DoModal();	

	return TRUE;
}

BOOL CMXFInspectorDlg::DumpEntry(unsigned __int64 offset)
{
	m_treeCtrl.DeleteAllItems();

	HTREEITEM root = 0;	

	unsigned char keyValue[16];
	unsigned char ch;
	DWORD size, szSize, nRead;	
	HANDLE hMXF;

	hMXF = CreateFile(m_fnameMXF, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hMXF == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(L"Erro ao abrir arquivo MXF\n");
		return FALSE;
	}

	unsigned __int64 filePos;

	// seek
	LARGE_INTEGER pos = {0};
	DWORD ret;

	pos.QuadPart = offset;
	ret = SetFilePointer(hMXF, pos.LowPart, &pos.HighPart, FILE_CURRENT);
	if(ret == INVALID_SET_FILE_POINTER && GetLastError() != ERROR_SUCCESS)
	{		
		return FALSE;
	}

	filePos = GetFilePosition(hMXF);

	ReadFile(hMXF, keyValue, 16, &nRead, NULL);

	if(nRead < 16)
	{		
		return FALSE; // EOF
	}

	ASSERT(filePos == offset);

	ReadFile(hMXF, &ch, 1, &nRead, NULL);
	if( ch & 0x80 )
	{
		szSize = ch&0x0f;
		size = (DWORD)GetNumber(hMXF, szSize);
	}
	else
	{
		size = ch;
	}
	
	if(!memcmp(keyValue, headerKey, sizeof(headerKey)))
	{	
		m_treeCtrl.DeleteAllItems();
		root = m_treeCtrl.InsertItem(L"HEADER");

		int status = keyValue[14];

		CString str;
		str.Format(L"status = %x", status);		
		m_treeCtrl.InsertItem(str, root);

		PartitionPack pack;
		pack.Read(hMXF, size);
		DumpMXFPartitionPack(pack, root);
	}

	/*
	if(!memcmp(keyValue, cppictureItemKey, sizeof(cppictureItemKey)) ||
		!memcmp(keyValue, gcpictureItemKey, sizeof(gcpictureItemKey)) )
	{			
		LPBYTE pBuffer = new BYTE[size];
		ReadFile(hMXF, pBuffer, size, &nRead, NULL);

		FILE* fp;
		fp = fopen("./video.mpg", "ab");
		fwrite(pBuffer, size, sizeof(BYTE), fp);
		fclose(fp);

		delete []pBuffer;
	}
	*/

	if(!memcmp(keyValue, bodyKey, sizeof(bodyKey)))
	{
		m_treeCtrl.DeleteAllItems();
		root = m_treeCtrl.InsertItem(L"BODY");

		int status = keyValue[14];

		CString str;
		str.Format(L"status = %x", status);		
		m_treeCtrl.InsertItem(str, root);

		PartitionPack pack;
		pack.Read(hMXF, size);
		DumpMXFPartitionPack(pack, root);
	}	

	if(!memcmp(keyValue, primerPackKey, sizeof(primerPackKey)))
	{
		m_treeCtrl.DeleteAllItems();
		root = m_treeCtrl.InsertItem(L"PRIMER PACK");

		m_primerPack.Read(hMXF);
		DumpMXFPrimerPack(m_primerPack, root);		
	}

	if(!memcmp(keyValue, footerKey, sizeof(footerKey)))
	{
		m_treeCtrl.DeleteAllItems();
		root = m_treeCtrl.InsertItem(L"FOOTER");

		int status = keyValue[14];

		CString str;
		str.Format(L"status = %x", status);		
		m_treeCtrl.InsertItem(str, root);		

		PartitionPack pack;
		pack.Read(hMXF, size);
		DumpMXFPartitionPack(pack, root);		
	}

	/*if(!memcmp(keyValue, klvFillDataKey, sizeof(klvFillDataKey)))
	{
	root = m_treeCtrl.InsertItem(L"KLVFILL");		
	}*/

	/*if(!memcmp(keyValue, fillKey, sizeof(fillKey)))
	{
	root = m_treeCtrl.InsertItem(L"FILL");	
	}*/


	if(!memcmp(keyValue, metadataKey, sizeof(metadataKey)))
	{
		m_treeCtrl.DeleteAllItems();

		if( (keyValue[13]==0x01) && (keyValue[14] == 0x2f)) // PREFACE
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <Preface>");

			Preface p;
			p.Read(hMXF, size);
			DumpMXFPreface(p, root);		
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x30)) // IDENTIFICATION		
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <Identification>");
			Identification id;
			id.Read(hMXF, size);
			DumpMXFIdentification(id, root);			
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x18)) // CONTENT STORAGE
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <Content Storage>");
			ContentStorage cs;
			cs.Read(hMXF, size);
			DumpMXFContentStorage(cs, root);		
		}		
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x23)) // ESSENCE CONTAINER DATA
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <Essence Container Data>");
			EssenceContainerData ecd;
			ecd.Read(hMXF, size);
			DumpMXFEssenceContainerData(ecd, root);			
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x32)) // NETWORK LOCATOR
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <Network Locator>");
			/*GenericPackage gp;
			gp.Read(hMXF, size);
			DumpMXFGenericPackage(gp, root);*/			
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x33)) // TEXT LOCATOR
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <Text Locator>");
			/*GenericPackage gp;
			gp.Read(hMXF, size);
			DumpMXFGenericPackage(gp, root);*/			
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x34)) // GENERIC PACKAGE
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <Generic Package>");
			GenericPackage gp;
			gp.Read(hMXF, size);
			DumpMXFGenericPackage(gp, root);			
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x36)) // MATERIAL PACKAGE
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <Material Package>");
			GenericPackage gp;
			gp.Read(hMXF, size);
			DumpMXFGenericPackage(gp, root);			
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x38)) // GENERIC TRACK
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <Generic Track>");
			/*GenericTra Package gp;
			gp.Read(hMXF, size);
			DumpMXFGenericPackage(gp, root);			*/
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x39)) // EVENT TRACK
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <Event Track>");
			/*Track t;
			t.Read(hMXF);
			DumpMXFTrack(t, root);	*/		
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x3a)) // STATIC TRACK
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <Static Track>");
			StaticTrack t;
			t.Read(hMXF, size);
			DumpMXFStaticTrack(t, root);			
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x3b)) // TRACK
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <Track>");
			Track t;
			t.Read(hMXF, size);
			DumpMXFTrack(t, root);			
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x0f)) // SEQUENCE
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <Sequence>");
			Sequence seq;
			seq.Read(hMXF, size);
			DumpMXFMetadataSequence(seq, root);			
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x14)) // TIMECODE COMPONENT
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <Timecode Component>");
			TimecodeComponent tc;
			tc.Read(hMXF, size);
			DumpMXFMetadataTimecodeComponent(tc, root);			
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x11)) // SOURCE CLIP
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <Source Clip>");
			SourceClip clip;
			clip.Read(hMXF, size);
			DumpMXFMetadataSourceClip(clip, root);			
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x41)) // DM Segment
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <DM Segment>");
			DMSegment dms;
			dms.Read(hMXF, size);
			DumpMXFDMSegment(dms, root);			
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x45)) // DMSource Clip
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <DMSource Clip>");
			/*DMSegment dms;
			dms.Read(hMXF, size);
			DumpMXFDMSegment(dms, root);*/			
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x37)) // SOURCE PACKAGE
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <Source Package>");
			SourcePackage sp;
			sp.Read(hMXF, size);
			DumpMXFSourcePackage(sp, root);			
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x25)) // FILE DESCRIPTOR
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <File Descriptor>");
			/*MultipleDescriptor md;
			md.Read(hMXF, size);
			DumpMXFMultipleDescriptor(md, root);			*/
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x27)) // GENERIC PICTURE ESSENCE DESCRIPTOR
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <Generic Picture Essence Descriptor>");
			/*CDCIPictureEssenceDescriptor cped;
			cped.Read(hMXF, size);
			DumpMXFCDCIPictureEssenceDescriptor(cped, root);		*/	
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x28)) // CDCI PICTURE ESSENCE DESCRIPTOR
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <CDCI Picture Essence Descriptor>");
			CDCIPictureEssenceDescriptor cped;
			cped.Read(hMXF, size);
			DumpMXFCDCIPictureEssenceDescriptor(cped, root);			
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x29)) // RGBA ESSENCE DESCRIPTOR
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <RGBA Essence Descriptor>");
			/*CDCIPictureEssenceDescriptor cped;
			cped.Read(hMXF, size);
			DumpMXFCDCIPictureEssenceDescriptor(cped, root);*/			
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x42)) // GENERIC SOUND ESSENCE DESCRIPTOR
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <Generic Sound Essence Descriptor>");
			GenericSoundEssenceDescriptor gsed;
			gsed.Read(hMXF, size);
			DumpMXFGenericSoundEssenceDescriptor(gsed, root);				
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x43)) // GENERIC DATA ESSENCE DESCRIPTOR
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <Generic Data Essence Descriptor>");
			/*GenericSoundEssenceDescriptor gsed;
			gsed.Read(hMXF, size);
			DumpMXFGenericSoundEssenceDescriptor(gsed, root);	*/		
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x5b)) // VBI DATA DESCRIPTOR
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <VBI Data Descriptor>");
			/*GenericSoundEssenceDescriptor gsed;
			gsed.Read(hMXF, size);
			DumpMXFGenericSoundEssenceDescriptor(gsed, root);	*/		
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x44)) // MULTIPLE DESCRIPTOR
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <Multiple Descriptor>");
			MultipleDescriptor md;
			md.Read(hMXF, size);
			DumpMXFMultipleDescriptor(md, root);			
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x51)) // MPEG2 VIDEO DESCRIPTOR
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <MPEG2 Video Descriptor>");
			MPEG2VideoDescriptor mpg2VD;
			mpg2VD.Read(hMXF, &m_primerPack, size);
			DumpMXFMPEG2VideoDescriptor(mpg2VD, root);			
		}
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x48)) // WAVE AUDIO ESSENCE DESCRIPTOR
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <Wave Audio Essence Descriptor>");
			WaveAudioEssenceDescriptor waveAED;
			waveAED.Read(hMXF, size);
			DumpMXFWaveAudioEssenceDescriptor(waveAED, root);				
		}		
		if( (keyValue[13]==0x01) && (keyValue[14] == 0x47)) // AES3 AUDIO ESSENCE DESCRIPTOR
		{
			root = m_treeCtrl.InsertItem(L"Metadata: <AES3 Audio Essence Descriptor>");
			AES3AudioEssenceDescriptor aes3AED;
			aes3AED.Read(hMXF, size);
			DumpMXFAES3AudioEssenceDescriptor(aes3AED, root);				
		}				
	}

	if(!memcmp(keyValue, indexTableKey, sizeof(indexTableKey)))
	{
		m_treeCtrl.DeleteAllItems();
		root = m_treeCtrl.InsertItem(L"INDEX TABLE");	

		IndexTableSegment its;
		its.Read(hMXF, size);

		DumpMXFIndexTable(its, root);		
	}

	if(!memcmp(keyValue, systemMetadataPackKey, sizeof(systemMetadataPackKey)))
	{
		m_treeCtrl.DeleteAllItems();
		root = m_treeCtrl.InsertItem(L"SYSTEM METADATA PACK");	

		SystemMetadataPack smp;
		smp.Read(hMXF);
		DumpMXFSystemMetadataPack(smp, root);		
	}

	if(!memcmp(keyValue, packageMetadataSetKey, sizeof(packageMetadataSetKey)))
	{
		m_treeCtrl.DeleteAllItems();
		root = m_treeCtrl.InsertItem(L"PACKAGE METADATA SET");	

		int blockCount = keyValue[15];
		CString str;
		str.Format(L"MetadataBlockCount = %d", blockCount);
		m_treeCtrl.InsertItem(str, root);
	}

	if(!memcmp(keyValue, xmlDocumentTextKey, sizeof(xmlDocumentTextKey)))
	{
		m_treeCtrl.DeleteAllItems();
		root = m_treeCtrl.InsertItem(L"XMLDocument");

		CString str = L"";

		char* xml = new char[size+1];
		memset(xml, 0, size+1);
		BOOL ret = ReadFile(hMXF, xml, size, &nRead, NULL);
		//xml[size+1] = 0;

		/*for(unsigned int i = 0; i < size; i++)
		{
			char ch;
			DWORD nRead;
			ReadFile(hMXF, &ch, 1, &nRead, NULL);
			str += ch;
		}*/

		str = CString(xml);		
		m_treeCtrl.InsertItem(str, root);
		delete []xml;
	}

	if(!memcmp(keyValue, randomIndexPackKey, sizeof(randomIndexPackKey)))
	{
		m_treeCtrl.DeleteAllItems();
		root = m_treeCtrl.InsertItem(L"RANDOM INDEX PACK");

		RandomIndexPack rip;
		rip.Read(hMXF, size);
		DumpMXFRandomIndexPack(rip, root);
	}	

	m_treeCtrl.Expand(root, TVE_EXPAND);	

	return TRUE;
}

BOOL CMXFInspectorDlg::DumpMXFPartitionPack(PartitionPack& ppack, HTREEITEM root)
{
	HTREEITEM item = 0;
	CString str;

	str.Format(L"Major Version = %d", ppack.majorVersion);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"Minor Version = %d", ppack.minorVersion);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"KAG Size = %d", ppack.kagSize);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"This Partition = 0x%08x", ppack.thisPartition);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"Previous Partition = 0x%08x", ppack.previousPartition);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"Footer Partition = 0x%08x", ppack.footerPartition);
	m_treeCtrl.InsertItem(str, root);	

	str.Format(L"Header Byte Count = %d", ppack.headerByteCount);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"IndexByteCount = %d", ppack.indexByteCount);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"IndexSID = %d", ppack.indexSID);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"BodyOffset = 0x%x", ppack.bodyOffset);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"BodySID = %d", ppack.bodySID);
	m_treeCtrl.InsertItem(str, root);

	str = L"Operational Pattern  = ";
	for(int i = 0; i < 16; i++)
	{
		CString temp;
		temp.Format(L"%02x ", ppack.operationPattern[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);

	str = L"Essence Containers: ";
	item = m_treeCtrl.InsertItem(str, root);
	for(unsigned int i = 0; i < ppack.essenceContainers.num; i++)
	{
		str = L"";
		for(unsigned int j = 0; j < ppack.essenceContainers.size; j++)
		{
			CString temp;
			temp.Format(L"%02x ", ppack.essenceContainers.ppItem[i][j]);
			str += temp;
		}
		m_treeCtrl.InsertItem(str, item);
	}	

	return TRUE;
}

BOOL CMXFInspectorDlg::DumpMXFPrimerPack(PrimerPack& pp, HTREEITEM root)
{
	HTREEITEM item = 0;
	CString str;

	str.Format(L"NumberOfItems = %d", pp.localTagEntryBatch.numberOfItems);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"ItemLength = %d", pp.localTagEntryBatch.itemLength);
	m_treeCtrl.InsertItem(str, root);

	for(unsigned int i = 0; i < pp.localTagEntryBatch.numberOfItems; i++)
	{	
		str.Format(L"0x%04x => ", pp.localTagEntryBatch.pItems[i].localTag);

		for(unsigned int j = 0; j < 16; j++)
		{
			CString temp;
			temp.Format(L"%02x ", pp.localTagEntryBatch.pItems[i].uid[j]);
			str += temp;			
		}

		m_treeCtrl.InsertItem(str, root);	
	}

	return TRUE;
}

BOOL CMXFInspectorDlg::DumpMXFSystemMetadataPack(SystemMetadataPack& smp, HTREEITEM root)
{
	int i =0;
	HTREEITEM item = 0;
	CString str;

	str.Format(L"SystemMetadataBitmap = %d", smp.systemMetadataBitmap);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"ContentPackageRate = %d", smp.contentPackageRate);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"ContentPackageType = %d", smp.contentPackageType);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"ChannelHandle = %d", smp.channelHandle);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"ContinuityCount = %d", smp.continuityCount);
	m_treeCtrl.InsertItem(str, root);

	str = L"UniversalLabel = ";
	for(i = 0; i < 16; i++)
	{
		CString temp;
		temp.Format(L"%02x ", smp.universalLabel[i]);
		str += temp;		
	}
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"CreationDateTimeStamp:");
	item = m_treeCtrl.InsertItem(str, root);

	str.Format(L"Type = 0x%02x", smp.creationDateTimeStamp.type);
	m_treeCtrl.InsertItem(str, item);

	str.Format(L"Data = ");	
	for(i = 0; i < 16; i++)
	{
		CString temp;
		temp.Format(L"%02x ", smp.creationDateTimeStamp.data[i]);
		str += temp;	
	}
	m_treeCtrl.InsertItem(str, item);

	if(smp.creationDateTimeStamp.type == 0x81)
	{
		// Timecode
		// SMPTE 331
		// Seção 8.2
		SMPTE12MTimecode* tc = (SMPTE12MTimecode*)smp.creationDateTimeStamp.data;

		str.Format(L"Timecode = %02d:%02d:%02d:%02d", 10*tc->hoursTens+tc->hoursUnits,
			10*tc->minutesTens+tc->minutesUnits, 
			10*tc->secondsTens+tc->secondsUnits, 
			10*tc->frameTens+tc->frameUnits);

		m_treeCtrl.InsertItem(str, item);
	}

	str.Format(L"UserDateTimeStamp:");
	item = m_treeCtrl.InsertItem(str, root);

	str.Format(L"Type = 0x%02x", smp.userDateTimeStamp.type);
	m_treeCtrl.InsertItem(str, item);

	str.Format(L"Data = ");	
	for(i = 0; i < 16; i++)
	{
		CString temp;
		temp.Format(L"%02x ", smp.userDateTimeStamp.data[i]);
		str += temp;	
	}
	m_treeCtrl.InsertItem(str, item);

	if(smp.userDateTimeStamp.type == 0x81)
	{
		// Timecode
		// SMPTE 331
		// Seção 8.2
		SMPTE12MTimecode* tc = (SMPTE12MTimecode*)smp.userDateTimeStamp.data;
		str.Format(L"Color Frame Flag (CF) = %d", tc->cfFlag);
		m_treeCtrl.InsertItem(str, item);

		str.Format(L"Drop Frame Flag (DF) = %d", tc->dfFlag);
		m_treeCtrl.InsertItem(str, item);

		str.Format(L"Field Phase (FP) = %d", tc->fp);
		m_treeCtrl.InsertItem(str, item);

		str.Format(L"Timecode = %02d:%02d:%02d:%02d", 10*tc->hoursTens+tc->hoursUnits,
													  10*tc->minutesTens+tc->minutesUnits, 
													  10*tc->secondsTens+tc->secondsUnits, 
													  10*tc->frameTens+tc->frameUnits);
		m_treeCtrl.InsertItem(str, item);

		str.Format(L"BinaryGroup0(NTSC) / BinaryGroup2(PAL) (B0) = %d", tc->b0);
		m_treeCtrl.InsertItem(str, item);

		str.Format(L"BinaryGroup2(NTSC) / FieldPhase(PAL) (B1) = %d", tc->b1);
		m_treeCtrl.InsertItem(str, item);

		str.Format(L"BinaryGroup1 (B2) = %d", tc->b2);
		m_treeCtrl.InsertItem(str, item);

		str.Format(L"BinaryGroupData = 0x%02x 0x%02x 0x%02x 0x%02x", tc->bg1, tc->bg2, tc->bg3, tc->bg4); 
		m_treeCtrl.InsertItem(str, item);
	}

	return TRUE;
}

BOOL CMXFInspectorDlg::DumpMXFIndexTable(IndexTableSegment& its, HTREEITEM root)
{
	unsigned int i;
	HTREEITEM item = 0;
	CString str, temp;

	str.Format(L"InstanceUID = ");
	for(i = 0; i < 16; i++)
	{
		temp.Format(L"%02x ", its.instanceID[i]);
		str += temp;		
	}
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"IndexEditRate = %d/%d", its.indexEditRate.num, its.indexEditRate.den);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"IndexStartPosition = %I64d", its.indexStartPosition);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"IndexDuration = %I64d", its.indexDuration);
	m_treeCtrl.InsertItem(str, root);

	if(its.bEditUnitByteCount)			
		str.Format(L"EditUnitByteCount = %d", its.editUnitByteCount);	
	else
		str.Format(L"EditUnitByteCount = (encoder optional and not specified)");		
	m_treeCtrl.InsertItem(str, root);

	if(its.bIndexSID)
		str.Format(L"IndexSID = %d", its.indexSID);
	else
		str.Format(L"IndexSID = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"BodySID = %d", its.bodySID);
	m_treeCtrl.InsertItem(str, root);

	if(its.bSliceCount)
		str.Format(L"SliceCount = %d", its.sliceCount);	
	else
		str.Format(L"SliceCount = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(its.bPosTableCount)
		str.Format(L"PosTableCount = %d", its.posTableCount);
	else
		str.Format(L"PosTableCount = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);


	if(its.bDeltaEntryArray)
	{
		str.Format(L"DeltaEntryArray:");
		item = m_treeCtrl.InsertItem(str, root);

		str.Format(L"NDE = %d", its.deltaEntryArray.NDE);
		m_treeCtrl.InsertItem(str, item);

		str.Format(L"Length = %d", its.deltaEntryArray.length);
		m_treeCtrl.InsertItem(str, item);

		for(unsigned int i = 0; i < its.deltaEntryArray.NDE; i++)
		{
			str.Format(L"DeltaEntry (%d)", i);
			HTREEITEM child = m_treeCtrl.InsertItem(str, item);

			str.Format(L"PosTableIndex = %d", its.deltaEntryArray.pDeltaEntry[i].posTableIndex);
			m_treeCtrl.InsertItem(str, child);

			str.Format(L"Slice = %d", its.deltaEntryArray.pDeltaEntry[i].slice);
			m_treeCtrl.InsertItem(str, child);

			str.Format(L"ElementDelta = %d", its.deltaEntryArray.pDeltaEntry[i].elementDelta);
			m_treeCtrl.InsertItem(str, child);			
		}
	}
	else
	{
		str.Format(L"DeltaEntryArray = (optional and not specified)");
		m_treeCtrl.InsertItem(str, root);
	}

	if(its.bIndexEntryArray)
	{
		str.Format(L"IndexEntryArray:");
		item = m_treeCtrl.InsertItem(str, root);

		str.Format(L"NDE = %d", its.indexEntryArray.NIE);
		m_treeCtrl.InsertItem(str, item);

		str.Format(L"Length = %d", its.indexEntryArray.length);
		m_treeCtrl.InsertItem(str, item);

		for(unsigned int i = 0; i < its.indexEntryArray.NIE; i++)
		{
			str.Format(L"IndexEntry (%d)", i);
			HTREEITEM child = m_treeCtrl.InsertItem(str, item);

			if( its.indexEntryArray.pIndexEntry[i].temporalOffset )
				str.Format(L"TemporalOffset = %+d", its.indexEntryArray.pIndexEntry[i].temporalOffset);
			else
				str.Format(L"TemporalOffset = %d", its.indexEntryArray.pIndexEntry[i].temporalOffset);
			m_treeCtrl.InsertItem(str, child);

			str.Format(L"KeyFrameOffset = %d", its.indexEntryArray.pIndexEntry[i].keyFrameOffset);
			m_treeCtrl.InsertItem(str, child);
			
			unsigned char flag = its.indexEntryArray.pIndexEntry[i].flags;
			switch( flag )
			{
				case 0xC0:
					str.Format(L"Flags = %#02X. I frame (no prediction)", flag);
				break;
				case 0x22:
					str.Format(L"Flags = %#02X. P frame (forward prediction)", flag);
				break;
				case 0x33:
					str.Format(L"Flags = %#02X. B frame (forward & backward prediction)", flag);
				break;
				case 0x13:
					str.Format(L"Flags = %#02X. B frame (backward prediction)", flag);
				break;
			}			
			m_treeCtrl.InsertItem(str, child);

			str.Format(L"StreamOffset = %I64x", its.indexEntryArray.pIndexEntry[i].streamOffset);
			m_treeCtrl.InsertItem(str, child);

			if(its.bSliceCount && (its.sliceCount > 0))
			{
				for(unsigned int j=0; j < its.sliceCount; j++)
				{
					str.Format(L"SliceOffset (%d) = %x", j, its.indexEntryArray.pIndexEntry[i].pSliceOffset[j]);
					m_treeCtrl.InsertItem(str, child);
				}
			}

			if(its.bPosTableCount && (its.posTableCount > 0))
			{
				for(unsigned int j=0; j < its.sliceCount; j++)
				{
					str.Format(L"PosTableCount (%d) = %d/%d", j, its.indexEntryArray.pIndexEntry[i].pPosTable[j].num, 
						its.indexEntryArray.pIndexEntry[i].pPosTable[j].den);

					m_treeCtrl.InsertItem(str, child);
				}
			}			
		}
	}
	else
	{
		str.Format(L"IndexEntryArray = (optional and not specified)");
		m_treeCtrl.InsertItem(str, root);		
	}

	return TRUE;
}

BOOL CMXFInspectorDlg::DumpMXFRandomIndexPack(RandomIndexPack& rip, HTREEITEM root)
{	
	HTREEITEM item = 0;
	CString str;

	str.Format(L"Num = %d", rip.numElements);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"Length = %d", rip.overallLength);
	m_treeCtrl.InsertItem(str, root);

	for(unsigned int i = 0; i < rip.numElements; i++)
	{
		str.Format(L"Pack #%d", i);
		item = m_treeCtrl.InsertItem(str, root);

		str.Format(L"BodySID = %d", rip.pElements[i].bodySID);
		m_treeCtrl.InsertItem(str, item);

		str.Format(L"ByteOffset = %I64x", rip.pElements[i].byteOffset);
		m_treeCtrl.InsertItem(str, item);		
	}


	return TRUE;
}


void CMXFInspectorDlg::DumpMXFDMSegment(DMSegment& dms, HTREEITEM root)
{
	unsigned int i, j;
	HTREEITEM item = 0;
	CString str, temp;


	str.Format(L"InstanceUID = ");
	for(i = 0; i < 16; i++)
	{
		temp.Format(L"%02x ", dms.instanceUID[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);	

	str.Format(L"GenerationUID = ");
	if(dms.bGenerationUID)
	{
		for(i = 0; i < 16; i++)
		{		
			temp.Format(L"%02x ", dms.generationUID[i]);
			str += temp;
		}		
	}
	else
		str += L" = (optional and not specified)";

	m_treeCtrl.InsertItem(str, root);

	str.Format(L"Data Definition = ");
	for(i = 0; i < 16; i++)
	{
		temp.Format(L"%02x ", dms.dataDefinition[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);


	if(dms.bEventStartPosition)
		str.Format(L"Event Start Position = %I64d", dms.eventStartPosition);	
	else	
		str.Format(L"Event Start Position = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(dms.bDuration)
		str.Format(L"Duration = %I64d", dms.duration);
	else
		str.Format(L"Duration = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(dms.bEventComment)
		str.Format(L"Event Comment = %s", dms.eventComment);
	else
		str.Format(L"Event Comment = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(dms.bTrackID)
	{
		str.Format(L"Track IDs:");
		item = m_treeCtrl.InsertItem(str, root);

		str.Format(L"Num = %d", dms.trackID.num);
		m_treeCtrl.InsertItem(str, item);

		str.Format(L"Size = %d", dms.trackID.size);
		m_treeCtrl.InsertItem(str, item);

		for(i = 0; i < dms.trackID.num; i++)
		{
			str.Format(L"Track ID (%d) = ", i);			

			for(j = 0; j < dms.trackID.size; j++)
			{
				temp.Format(L"%02x ", dms.trackID.ppItem[i][j]);
				str += temp;
			}			
			m_treeCtrl.InsertItem(str, item);
		}		
	}
	else
	{
		str.Format(L"TrackIDs = (optional and not specified)");
		m_treeCtrl.InsertItem(str, root);
	}

	if(dms.bDMFramework)
	{
		str.Format(L"DMFramework = ");

		for(i = 0; i < 16; i++)
		{
			temp.Format(L"%02x ", dms.DMFramework[i]);
			str += temp;
		}

		m_treeCtrl.InsertItem(str, root);
	}
	else
	{		
		str.Format(L"DMFramework = (optional and not specified)");
		m_treeCtrl.InsertItem(str, root);
	}
}

void CMXFInspectorDlg::DumpMXFMPEG2VideoDescriptor(MPEG2VideoDescriptor& vd, HTREEITEM root)
{	
	CString str;

	DumpMXFCDCIPictureEssenceDescriptor(vd.cdciPictureEssenceDescriptor, root);

	if(vd.bSingleSequence)
		str.Format(L"SingleSequence = %d", vd.singleSequence);	
	else
		str.Format(L"SingleSequence = (encoder optional and not specified)");	
	m_treeCtrl.InsertItem(str, root);

	if(vd.bConstantBFrames)
		str.Format(L"ConstantBFrames = %d", vd.constantBFrames);
	else
		str.Format(L"ConstantBFrames = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(vd.bCodedContentType)
	{
		switch(vd.codedContentType)
		{
		case 0:				
			str.Format(L"CodedContentType = 0 (Unknown)");
			break;
		case 1:				
			str.Format(L"CodedContentType = 1 (Progressive)");
			break;
		case 2:				
			str.Format(L"CodedContentType = 2 (Interlaced)");
			break;
		case 3:				
			str.Format(L"CodedContentType = 3 (Mixed)");
			break;
		default:				
			str.Format(L"CodedContentType = %d (valor não permitido)", vd.codedContentType);
		}
	}
	else
		str.Format(L"CodedContentType = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(vd.bLowDelay)
		str.Format(L"LowDelay = %d", vd.lowDelay);	
	else
		str.Format(L"LowDelay = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);


	if(vd.bClosedGOP)
		str.Format(L"ClosedGOP = %d", vd.closedGOP);
	else
		str.Format(L"ClosedGOP = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(vd.bIdenticalGOP)
		str.Format(L"IdenticalGOP = %d", vd.identicalGOP);	
	else	
		str.Format(L"IdenticalGOP = (encoder optional and not specified)");	
	m_treeCtrl.InsertItem(str, root);

	if(vd.bMaxGOP)	
		str.Format(L"MaxGOP = %d", vd.maxGOP);
	else
		str.Format(L"MaxGOP = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(vd.bBPictureCount)
		str.Format(L"BPictureCount = %d", vd.bPictureCount);
	else
		str.Format(L"BPictureCount = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(vd.bBitRate)		
		str.Format(L"BitRate = %d", vd.bitRate);
	else
		str.Format(L"BitRate = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(vd.bProfileAndLevel)
		str.Format(L"ProfileAndLevel = 0x%02x", vd.profileAndLevel);	
	else
		str.Format(L"ProfileAndLevel = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);
}



void CMXFInspectorDlg::DumpMXFWaveAudioEssenceDescriptor(WaveAudioEssenceDescriptor& wd, HTREEITEM root)
{
	CString str, temp;

	DumpMXFGenericSoundEssenceDescriptor(wd.genericSoundEssenceDescriptor, root);

	str.Format(L"BlockAlign = %d", wd.blockAlign);
	m_treeCtrl.InsertItem(str, root);

	if(wd.bSequenceOffset)
		str.Format(L"Sequence Offset = %d", wd.sequenceOffset);	
	else		
		str.Format(L"Sequence Offset = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"AvgBps = %d", wd.avgBps);
	m_treeCtrl.InsertItem(str, root);


	str.Format(L"ChannelAssignment = ");
	if(wd.bChannelAssignment)
	{	
		for(int i = 0; i < 16; i++)
		{
			temp.Format(L"%02x ", wd.channelAssignment[i]);
			str += temp;
		}	
	}
	else	
		str += L"(encoder optional and not specified)";	
	m_treeCtrl.InsertItem(str, root);

	if(wd.bPeakEnvelopeVersion)
		str.Format(L"PeakEnvelopVersion = %d", wd.peakEnvelopeVersion);	
	else
		str.Format(L"PeakEnvelopVersion = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(wd.bPeakEnvelopeFormat)
		str.Format(L"PeakEnvelopFormat = %d", wd.peakEnvelopeFormat);
	else
		str.Format(L"PeakEnvelopFormat = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(wd.bPointsPerPeakValue)
		str.Format(L"PointsPerPeakValue = %d", wd.pointsPerPeakValue);
	else
		str.Format(L"PointsPerPeakValue = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(wd.bPeakEnvelopeBlockSize)
		str.Format(L"PeakEnvelopBlockSize = %d", wd.peakEnvelopeBlockSize);
	else
		str.Format(L"PeakEnvelopBlockSize = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(wd.bPeakChannels)
		str.Format(L"PeakChannels = %d", wd.peakChannels);
	else
		str.Format(L"PeakChannels = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(wd.bPeakFrames)
		str.Format(L"PeakFrames = %d", wd.peakFrames);
	else
		str.Format(L"PeakFrames = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(wd.bPeakOfPeaksPosition)
		str.Format(L"PeakOfPeaksPosition = %d", wd.peakOfPeaksPosition);
	else
		str.Format(L"PeakOfPeaksPosition = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(wd.bPeakEnvelopeTimestamp)
	{
		str.Format(L"PeakEnvelopTimestamp = %02d/%02d/%04d - %02d:%02d:%02d-%02dms/4", 
			wd.peakEnvelopeTimestamp.st.day, 
			wd.peakEnvelopeTimestamp.st.month, 
			wd.peakEnvelopeTimestamp.st.year, 
			wd.peakEnvelopeTimestamp.st.hour, 
			wd.peakEnvelopeTimestamp.st.minute, 
			wd.peakEnvelopeTimestamp.st.second, 
			wd.peakEnvelopeTimestamp.st.quartermsec);
	}
	else
	{
		str.Format(L"PeakEnvelopTimestamp = (encoder optional and not specified)");
	}
	m_treeCtrl.InsertItem(str, root);

	if(wd.bPeakEnvelopeData)
		str.Format(L"Falta fazer o dump!!!");
	else
		str.Format(L"PeakEnvelopeData = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);
}

void CMXFInspectorDlg::DumpMXFAES3AudioEssenceDescriptor(AES3AudioEssenceDescriptor &ad, HTREEITEM root)
{
	unsigned int i, j;
	HTREEITEM item = 0;
	CString str, temp;

	DumpMXFWaveAudioEssenceDescriptor(ad.waveAudioEssenceDescriptor, root);

	
	if(ad.bEmphasis)
		str.Format(L"Emphasis = %d", ad.emphasis);	
	else
		str.Format(L"Emphasis = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);
	
	if(ad.bBlockStartOffset)
		str.Format(L"BlockStartOffset = %d", ad.blockStartOffset);
	else
		str.Format(L"BlockStartOffset = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);
	
	if(ad.bAuxBitsMode)
		str.Format(L"AuxBitsMode = %d", ad.auxBitsMode);
	else
		str.Format(L"AuxBitsMode = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);
	
	if(ad.bChannelStatusMode)
	{
		str.Format(L"ChannelStatusMode:");
		item = m_treeCtrl.InsertItem(str, root);

		str.Format(L"Num = %d", ad.channelStatusMode.num);
		m_treeCtrl.InsertItem(str, item);

		str.Format(L"Size = %d", ad.channelStatusMode.size);
		m_treeCtrl.InsertItem(str, item);
	
		for(i = 0; i < ad.channelStatusMode.num; i++)
		{
			switch(ad.channelStatusMode.pElements[i])
			{
			case 0x00: str.Format(L"Channel %d = No channel status data is encoded", i); break;
			case 0x01: str.Format(L"Channel %d = AES3 Minimum (byte 0 bit 0 = \'1\')", i); break;
			case 0x02: str.Format(L"Channel %d = AES3 Standard", i); break;
			case 0x03: str.Format(L"Channel %d = Fixed 24 byes of data in FixedChannelStatusData property", i); break;
			case 0x04: str.Format(L"Channel %d = Stream of data within MXF Header Metadata", i); break;
			case 0x05: str.Format(L"Channel %d = Stream of data multiplexed within MXF Body", i); break;			
			default:
				str.Format(L"Channel %d = %d", i, ad.channelStatusMode.pElements[i]); break;
			}	

			m_treeCtrl.InsertItem(str, item);
		}
	}
	else
	{
		str.Format(L"ChannelStatusMode = (encoder optional and not specified)");
		m_treeCtrl.InsertItem(str, root);
	}
	

	if(ad.bFixedChannelStatusData)
	{
		str.Format(L"FixedChannelStatusData:");
		item = m_treeCtrl.InsertItem(str, root);

		str.Format(L"Num = %d", ad.fixedChannelStatusData.num);
		m_treeCtrl.InsertItem(str, item);
		str.Format(L"Size = %d", ad.fixedChannelStatusData.size);
		m_treeCtrl.InsertItem(str, item);
	
		for(i = 0; i < ad.fixedChannelStatusData.num; i++)
		{
			str.Format(L"Channel %d =>", i);
			for(j = 0; j < ad.fixedChannelStatusData.size; j++)
			{
				temp.Format(L"%02x ", ad.fixedChannelStatusData.ppItem[i][j]);
				str += temp;
			}	
			m_treeCtrl.InsertItem(str, item);
		}
	}
	else
	{
		str.Format(L"FixedChannelStatusData = (encoder optional and not specified)");
		m_treeCtrl.InsertItem(str, root);
	}
	

	if(ad.bUserDataMode)
	{
		str.Format(L"UserDataMode:");
		item = m_treeCtrl.InsertItem(str, root);

		str.Format(L"Num = %d", ad.userDataMode.num);
		m_treeCtrl.InsertItem(str, item);
		str.Format(L"Size = %d", ad.userDataMode.size);
		m_treeCtrl.InsertItem(str, item);
	
		for(i = 0; i < ad.userDataMode.num; i++)
		{
			int v = ad.userDataMode.pElements[i];
			str.Format(L"UserData %d => %d", i, v);
			m_treeCtrl.InsertItem(str, item);
		}
	}
	else
	{
		str.Format(L"UserDataMode = (encoder optional and not specified)");
		m_treeCtrl.InsertItem(str, root);
	}
	

	if(ad.bFixedUserData)
	{
		str.Format(L"FixedUserData:");
		item = m_treeCtrl.InsertItem(str, root);

		str.Format(L"Num = %d", ad.fixedUserData.num);
		m_treeCtrl.InsertItem(str, item);
		str.Format(L"Size = %d", ad.fixedUserData.size);
		m_treeCtrl.InsertItem(str, item);
	
		for(i = 0; i < ad.fixedUserData.num; i++)
		{
			str.Format(L"UserData %d =>", i);
			for(j = 0; j < ad.fixedUserData.size; j++)
			{				
				temp.Format(L"%02x ", ad.fixedUserData.ppItem[i][j]);
				str += temp;
			}
			m_treeCtrl.InsertItem(str, item);
		}
	}
	else
	{
		str.Format(L"FixedUserData = (encoder optional and not specified)");
		m_treeCtrl.InsertItem(str, root);
	}
}


void CMXFInspectorDlg::DumpMXFPreface(Preface& p, HTREEITEM root)
{
	unsigned int i, j;
	HTREEITEM item = 0;
	CString str, temp;

	str.Format(L"InstanceUID = ");
	for(i = 0; i < 16; i++)
	{
		temp.Format(L"%02x ", p.instanceUID[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"GenerationUID = ");
	if(p.bGenerationUID)
	{
		for(i = 0; i < 16; i++)
		{
			temp.Format(L"%02x ", p.generationUID[i]);
			str += temp;
		}				
	}
	else
		str += " (optional and not specified)";
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"LastModifiedDate = %02d/%02d/%04d - %02d:%02d:%02d-%02dms/4", 
		p.lastModifiedDate.st.day, 
		p.lastModifiedDate.st.month, 
		p.lastModifiedDate.st.year,
		p.lastModifiedDate.st.hour, 
		p.lastModifiedDate.st.minute, 
		p.lastModifiedDate.st.second, 
		p.lastModifiedDate.st.quartermsec);
	m_treeCtrl.InsertItem(str, root);
	
	str.Format(L"Version = %d", p.version);
	m_treeCtrl.InsertItem(str, root);

	if(p.bObjectModelVersion)
		str.Format(L"ObjectModelVersion = %d", p.objectModelVersion);
	else
		str.Format(L"ObjectModelVersion = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"PrimaryPackage  = ");
	if(p.bPrimaryPackage)
	{
		for(i = 0; i < 16; i++)
		{
			temp.Format(L"%02x ", p.primaryPackage[i]);
			str += temp;
		}
	}
	else
		str += " (optional and not specified)";
	m_treeCtrl.InsertItem(str, root);
	
	str.Format(L"Identifications:");
	item = m_treeCtrl.InsertItem(str, root);

	str.Format(L"Num = %d", p.identifications.num);
	m_treeCtrl.InsertItem(str, item);

	str.Format(L"Size = %d", p.identifications.size);
	m_treeCtrl.InsertItem(str, item);

	for(i = 0; i < p.identifications.num; i++)
	{
		str.Format(L"Identification %d = ", i);
		for(j = 0; j < p.identifications.size; j++)
		{
			temp.Format(L"%02x ", p.identifications.ppItem[i][j]);						
			str += temp;
		}
		m_treeCtrl.InsertItem(str, item);
	}
	
	str.Format(L"ContentStorage = ");
	for(i = 0; i < 16; i++)
	{
		temp.Format(L"%02x ", p.contentStorage[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"Operational Pattern = ");
	for(i = 0; i < 16; i++)
	{
		temp.Format(L"%02x ", p.operationalPattern[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"Essence Containers:");
	item = m_treeCtrl.InsertItem(str, root);

	str.Format(L"Num = %d", p.essenceContainers.num);
	m_treeCtrl.InsertItem(str, item);

	str.Format(L"Size = %d", p.essenceContainers.size);
	m_treeCtrl.InsertItem(str, item);

	for(i = 0; i < p.essenceContainers.num; i++)
	{
		str.Format(L"Essence Container %d = ", i);
		for(j = 0; j < p.essenceContainers.size; j++)
		{
			temp.Format(L"%02x ", p.essenceContainers.ppItem[i][j]);
			str += temp;
		}
		m_treeCtrl.InsertItem(str, item);
	}

	str.Format(L"DM Schemes:");
	item = m_treeCtrl.InsertItem(str, root);

	str.Format(L"Num = %d", p.DMSchemes.num);
	m_treeCtrl.InsertItem(str, item);

	str.Format(L"Size = %d", p.DMSchemes.size);
	m_treeCtrl.InsertItem(str, item);

	for(i = 0; i < p.DMSchemes.num; i++)
	{
		str.Format(L"DM Scheme %d = ", i);
		for(j = 0; j < p.DMSchemes.size; j++)
		{
			temp.Format(L"%02x ", p.DMSchemes.ppItem[i][j]);
			str += temp;
		}
		m_treeCtrl.InsertItem(str, item);
	}
}

void CMXFInspectorDlg::DumpMXFIdentification(Identification& id, HTREEITEM root)
{
	unsigned int i;	
	CString str, temp;

	str.Format(L"InstanceUID = ");
	for(i = 0; i < 16; i++)
	{
		temp.Format(L"%02x ", id.instanceUID[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);


	str.Format(L"ThisGenerationUID = ");
	for(i = 0; i < 16; i++)
	{
		temp.Format(L"%02x ", id.thisGenerationUID[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);	

	str.Format(L"Company Name = %s", id.companyName);
	m_treeCtrl.InsertItem(str, root);
	
	str.Format(L"Product Name = %s", id.productName);
	m_treeCtrl.InsertItem(str, root);

	if(id.bProductVersion)
		str.Format(L"Product Version = %d.%d.%d.%d.%d", id.productVersion[0], id.productVersion[1], id.productVersion[2], id.productVersion[3], id.productVersion[4]);
	else
		str.Format(L"Product Version = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);
	
	
	str.Format(L"Version String = %s", id.versionString);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"Product UID = ");
	for(i = 0; i < 16; i++)
	{
		temp.Format(L"%02x ", id.productUID[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);
	
	str.Format(L"ModificationDate = %02d/%02d/%04d - %02d:%02d:%02d-%02dms/4", 
		id.modificationDate.st.day, 
		id.modificationDate.st.month,
		id.modificationDate.st.year, 
		id.modificationDate.st.hour, 
		id.modificationDate.st.minute, 
		id.modificationDate.st.second, 
		id.modificationDate.st.quartermsec);
	m_treeCtrl.InsertItem(str, root);

	if(id.bToolkitVersion)
		str.Format(L"Toolkit Version = %d.%d.%d.%d.%d", id.toolkitVersion[0], id.toolkitVersion[1], id.toolkitVersion[2], id.toolkitVersion[3], id.toolkitVersion[4]);
	else
		str.Format(L"Toolkit Version = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);
	

	if(id.bPlatform)
		str.Format(L"Platform = %s", id.platform);
	else
		str.Format(L"Platform = (optional and not specified)");	
	m_treeCtrl.InsertItem(str, root);
}

void CMXFInspectorDlg::DumpMXFContentStorage(ContentStorage& cs, HTREEITEM root)
{
	unsigned int i, j;
	HTREEITEM item = 0;
	CString str, temp;

	str.Format(L"InstanceUID = ");
	for(i = 0; i < 16; i++)
	{
		str.Format(L"%02x ", cs.instanceUID[i]);
	}
	
	str.Format(L"GenerationUID = ");
	if(cs.bGenerationUID)
	{		
		for(i = 0; i < 16; i++)
		{
			temp.Format(L"%02x ", cs.generationUID[i]);
			str += temp;
		}
	}
	else
		str += " (optional and not specified)";
	m_treeCtrl.InsertItem(str, root);
	
	str.Format(L"Packages:");
	item = m_treeCtrl.InsertItem(str, root);
	
	str.Format(L"Num = %d", cs.packages.num);
	m_treeCtrl.InsertItem(str, item);
	
	str.Format(L"Size = %d", cs.packages.size);
	m_treeCtrl.InsertItem(str, item);
	
	for(i = 0; i < cs.packages.num; i++)
	{
		str.Format(L"Package %d = ", i);
		for(j = 0; j < cs.packages.size; j++)
		{
			temp.Format(L"%02x ", cs.packages.ppItem[i][j]);
			str += temp;
		}
		m_treeCtrl.InsertItem(str, item);
	}

	if(cs.bEssenceContainerData)
	{
		str.Format(L"Essence Container Data:");
		item = m_treeCtrl.InsertItem(str, root);
	
		str.Format(L"Num = %d", cs.essenceContainerData.num);
		m_treeCtrl.InsertItem(str, item);
		
		str.Format(L"Size = %d", cs.essenceContainerData.size);
		m_treeCtrl.InsertItem(str, item);
		
		for(i = 0; i < cs.essenceContainerData.num; i++)
		{
			str.Format(L"Essence Container Data %d = ", i);
			for(j = 0; j < cs.essenceContainerData.size; j++)
			{
				temp.Format(L"%02x ", cs.essenceContainerData.ppItem[i][j]);
				str += temp;
			}
			m_treeCtrl.InsertItem(str, item);
		}
	}
	else
	{
		str.Format(L"Essence Container Data = (optional and not specified)");
		m_treeCtrl.InsertItem(str, root);
	}
}

void CMXFInspectorDlg::DumpMXFEssenceContainerData(EssenceContainerData& ecd, HTREEITEM root)
{	
	int i = 0;
	CString str, temp;

	str.Format(L"InstanceUID = ");
	for(i = 0; i < 16; i++)
	{
		temp.Format(L"%02x ", ecd.instanceUID[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"Linked Package UID = ");
	for(i = 0; i < 32; i++)
	{
		temp.Format(L"%02x ", ecd.linkedPackageUID[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);
	
	str.Format(L"GenerationUID = ");
	if(ecd.bGenerationUID)
	{
		for(i = 0; i < 16; i++)
		{
			temp.Format(L"%02x ", ecd.generationUID[i]);
			str += temp;
		}		
	}
	else
		str += " (optional and not specified)";
	m_treeCtrl.InsertItem(str, root);
	
	if(ecd.bIndexSID)
		str.Format(L"IndexSID = %d", ecd.indexSID);	
	else
		str.Format(L"IndexSID = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);
	
	
	str.Format(L"BodySID = %d", ecd.bodySID);
	m_treeCtrl.InsertItem(str, root);
}

void CMXFInspectorDlg::DumpMXFGenericPackage(GenericPackage& gp, HTREEITEM root)
{
	unsigned int i, j;
	HTREEITEM item = 0;
	CString str, temp;
	
	str.Format(L"InstanceUID = ");
	for(i = 0; i < 16; i++)
	{		
		temp.Format(L"%02x ", gp.instanceUID[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);
		
	str.Format(L"Package UID = ");
	for(i = 0; i < 32; i++)
	{	
		temp.Format(L"%02x ", gp.packageUID[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);
	
	str.Format(L"GenerationUID = ");
	if(gp.bGenerationUID)
	{		
		for(i = 0; i < 16; i++)
		{
			temp.Format(L"%02x ", gp.generationUID[i]);
			str += temp;
		}
	}
	else
		str += "optional and not specified)";
	m_treeCtrl.InsertItem(str, root);
	
	if(gp.bName)
		str.Format(L"Name = %s", gp.name);
	else
		str.Format(L"Name = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);
	
	str.Format(L"PackageCreationDate = %02d/%02d/%04d - %02d:%02d:%02d-%02dms/4", gp.packageCreationDate.st.day, gp.packageCreationDate.st.month, gp.packageCreationDate.st.year, gp.packageCreationDate.st.hour, gp.packageCreationDate.st.minute, gp.packageCreationDate.st.second, gp.packageCreationDate.st.quartermsec);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"PackageModifiedDate = %02d/%02d/%04d - %02d:%02d:%02d-%02dms/4", gp.packageModifiedDate.st.day, gp.packageModifiedDate.st.month, gp.packageModifiedDate.st.year, gp.packageModifiedDate.st.hour, gp.packageModifiedDate.st.minute, gp.packageModifiedDate.st.second, gp.packageModifiedDate.st.quartermsec);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"Tracks:");
	item = m_treeCtrl.InsertItem(str, root);
	
	str.Format(L"Num = %d", gp.tracks.num);
	m_treeCtrl.InsertItem(str, item);
	
	str.Format(L"Size = %d", gp.tracks.size);
	m_treeCtrl.InsertItem(str, item);
		
	for(i = 0; i < gp.tracks.num; i++)
	{
		str.Format(L"Track %d = ", i);
		for(j = 0; j < gp.tracks.size; j++)
		{
			temp.Format(L"%02x ", gp.tracks.ppItem[i][j]);
			str += temp;
		}
		m_treeCtrl.InsertItem(str, item);
	}

	str.Format(L"Descriptor = ");
	for(i = 0; i < 16; i++)
	{		
		temp.Format(L"%02x ", gp.descriptor[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);	
}


void CMXFInspectorDlg::DumpMXFTrack(Track& t, HTREEITEM root)
{
	unsigned int i;
	HTREEITEM item = 0;
	CString str, temp;
	
	str.Format(L"InstanceUID = ");
	for(i = 0; i < 16; i++)
	{		
		temp.Format(L"%02x ", t.instanceUID[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"GenerationUID = ");
	if(t.bGenerationUID)
	{
		for(i = 0; i < 16; i++)
		{
			temp.Format(L"%02x ", t.generationUID[i]);
			str += temp;
		}
	}
	else
		str += "(optional and not specified)";
	m_treeCtrl.InsertItem(str, root);
	
	if(t.bTrackID)
		str.Format(L"TrackID = %d", t.trackID);
	else
		str.Format(L"TrackID = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);
	
	str.Format(L"TrackNumber = 0x%08x", t.trackNumber);
	m_treeCtrl.InsertItem(str, root);

	if(t.bTrackName)
		str.Format(L"TrackName = %s", t.trackName);
	else
		str.Format(L"TrackName = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);
	
	str.Format(L"EditRate = %d/%d", t.editRate.num, t.editRate.den);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"Origin = %I64d", t.origin);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"Sequence = ");
	for(i = 0; i < 16; i++)
	{
		temp.Format(L"%02x ", t.sequence[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);
}

void CMXFInspectorDlg::DumpMXFStaticTrack(StaticTrack& t, HTREEITEM root)
{
	unsigned int i;
	HTREEITEM item = 0;
	CString str, temp;

	str.Format(L"InstanceUID = ");
	for(i = 0; i < 16; i++)
	{		
		temp.Format(L"%02x ", t.instanceUID[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"GenerationUID = ");
	if(t.bGenerationUID)
	{
		for(i = 0; i < 16; i++)
		{
			temp.Format(L"%02x ", t.generationUID[i]);
			str += temp;
		}
	}
	else
		str += "(optional and not specified)";
	m_treeCtrl.InsertItem(str, root);


	if(t.bTrackID)
		str.Format(L"TrackID = %d", t.trackID);
	else
		str.Format(L"TrackID = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"TrackNumber = 0x%08x", t.trackNumber);
	m_treeCtrl.InsertItem(str, root);

	if(t.bTrackName)
		str.Format(L"TrackName = %s", t.trackName);
	else
		str.Format(L"TrackName = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"Sequence = ");
	for(i = 0; i < 16; i++)
	{
		temp.Format(L"%02x ", t.sequence[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);
}

void CMXFInspectorDlg::DumpMXFMetadataSourceClip(SourceClip& clip, HTREEITEM root)
{
	unsigned int i;
	HTREEITEM item = 0;
	CString str, temp;

	str.Format(L"InstanceUID = ");
	for(i = 0; i < 16; i++)
	{		
		temp.Format(L"%02x ", clip.instanceUID[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"GenerationUID = ");
	if(clip.bGenerationUID)
	{
		for(i = 0; i < 16; i++)
		{
			temp.Format(L"%02x ", clip.generationUID[i]);
			str += temp;
		}
	}
	else
		str += "(optional and not specified)";
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"Data Definition = ");
	for(i = 0; i < 16; i++)
	{
		temp.Format(L"%02x ", clip.dataDefinition[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);
	
	str.Format(L"Start Position = %I64d", clip.startPosition);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"Duration = %I64d", clip.duration);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"Source Package ID = ");
	if(clip.bSourcePackageID)
	{		
		for(i = 0; i < 32; i++)
		{
			temp.Format(L"%02x ", clip.sourcePackageID[i]);
			str += temp;
		}
	}
	else
		str += " (optional and not specified)";
	m_treeCtrl.InsertItem(str, root);
		
	str.Format(L"Source Track ID = %d", clip.sourceTrackID);
	m_treeCtrl.InsertItem(str, root);
}

void CMXFInspectorDlg::DumpMXFMetadataSequence(Sequence& seq, HTREEITEM root)
{
	unsigned int i, j;
	HTREEITEM item = 0;
	CString str, temp;

	str.Format(L"InstanceUID = ");
	for(i = 0; i < 16; i++)
	{		
		temp.Format(L"%02x ", seq.instanceUID[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"GenerationUID = ");
	if(seq.bGenerationUID)
	{
		for(i = 0; i < 16; i++)
		{
			temp.Format(L"%02x ", seq.generationUID[i]);
			str += temp;
		}
	}
	else
		str += "(optional and not specified)";
	m_treeCtrl.InsertItem(str, root);


	str.Format(L"Data Definition = ");
	for(i = 0; i < 16; i++)
	{
		temp.Format(L"%02x ", seq.dataDefinition[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"Duration = %I64d", seq.duration);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"Structural Components:");
	item = m_treeCtrl.InsertItem(str, root);

	str.Format(L"Num = %d", seq.structuralComponents.num);
	m_treeCtrl.InsertItem(str, item);

	str.Format(L"Size = %d", seq.structuralComponents.size);
	m_treeCtrl.InsertItem(str, item);

	for(i = 0; i < seq.structuralComponents.num; i++)
	{
		str.Format(L"Structural Components %d = ", i);
		for(j = 0; j < seq.structuralComponents.size; j++)
		{
			temp.Format(L"%02x ", seq.structuralComponents.ppItem[i][j]);
			str += temp;
		}		
		m_treeCtrl.InsertItem(str, item);
	}
	
}

void CMXFInspectorDlg::DumpMXFMetadataTimecodeComponent(TimecodeComponent tc, HTREEITEM root)
{
	unsigned int i;
	CString str, temp;

	str.Format(L"InstanceUID = ");
	for(i = 0; i < 16; i++)
	{		
		temp.Format(L"%02x ", tc.instanceUID[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"GenerationUID = ");
	if(tc.bGenerationUID)
	{
		for(i = 0; i < 16; i++)
		{
			temp.Format(L"%02x ", tc.generationUID[i]);
			str += temp;
		}
	}
	else
		str += "(optional and not specified)";
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"Data Definition = ");
	for(i = 0; i < 16; i++)
	{
		temp.Format(L"%02x ", tc.dataDefinition[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"Duration = %I64d", tc.duration);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"Rounded Timecode Base= %d", tc.roundedTimecodeBase);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"StartTimecode = %I64d", tc.startTimecode);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"DropFrame= %d", tc.dropFrame);
	m_treeCtrl.InsertItem(str, root);
}

void CMXFInspectorDlg:: DumpMXFSourcePackage(SourcePackage& sp, HTREEITEM root)
{	
	DumpMXFGenericPackage(sp.package, root);	
}

void CMXFInspectorDlg:: DumpMXFFileDescriptor(FileDescriptor& fd, HTREEITEM root)
{
	unsigned int i, j;
	HTREEITEM item = 0;
	CString str, temp;

	str.Format(L"InstanceUID = ");
	for(i = 0; i < 16; i++)
	{		
		temp.Format(L"%02x ", fd.instanceUID[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"GenerationUID = ");
	if(fd.bGenerationUID)
	{
		for(i = 0; i < 16; i++)
		{
			temp.Format(L"%02x ", fd.generationUID[i]);
			str += temp;
		}
	}
	else
		str += "(optional and not specified)";
	m_treeCtrl.InsertItem(str, root);


	if(fd.bLinkedTrackID)
		str.Format(L"LinkedTrackID = %d", fd.linkedTrackID);
	else
		str.Format(L"LinkedTrackID = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);	

	str.Format(L"SampleRate = %d/%d", fd.sampleRate.num, fd.sampleRate.den);
	m_treeCtrl.InsertItem(str, root);	

	if(fd.bContainerDuration)
		str.Format(L"ContainerDuration = %I64d", fd.containerDuration);
	else
		str.Format(L"ContainerDuration = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);	

	str.Format(L"EssenceContainer = ");
	for(i = 0; i < 16; i++)
	{
		temp.Format(L"%02x ", fd.essenceContainer[i]);
		str += temp;
	}
	m_treeCtrl.InsertItem(str, root);	

	str.Format(L"Codec = ");
	if(fd.bCodec)
	{
		for(i = 0; i < 16; i++)
		{
			temp.Format(L"%02x ", fd.codec[i]);
			str += temp;
		}
	}
	else
		str += "(optional and not specified)";
	m_treeCtrl.InsertItem(str, root);	

	if(fd.bLocators)
	{
		str.Format(L"Locators:");
		item = m_treeCtrl.InsertItem(str, root);	

		str.Format(L"Num = %d", fd.locators.num);
		m_treeCtrl.InsertItem(str, item);

		str.Format(L"Size = %d", fd.locators.size);
		m_treeCtrl.InsertItem(str, item);

		for(i = 0; i < fd.locators.num; i++)
		{
			str.Format(L"Locator %d = ", fd.locators.num);
			for(j = 0; j < fd.locators.size; j++)
			{
				temp.Format(L"%02x ", fd.locators.ppItem[i][j]);
				str += temp;
			}
			m_treeCtrl.InsertItem(str, item);
		}
	}
	else
	{
		str.Format(L"Locators = (optional and not specified)");
		m_treeCtrl.InsertItem(str, root);	
	}
}

void CMXFInspectorDlg::DumpMXFMultipleDescriptor(MultipleDescriptor& md, HTREEITEM root)
{
	unsigned int i, j;
	HTREEITEM item = 0;
	CString str, temp;

	DumpMXFFileDescriptor(md.fileDescriptor, root);

	str.Format(L"SubDescriptorUIDs:");
	item = m_treeCtrl.InsertItem(str, root);

	str.Format(L"Num = %d", md.subDescriptorUIDs.num);
	m_treeCtrl.InsertItem(str, item);

	str.Format(L"Size = %d", md.subDescriptorUIDs.size);
	m_treeCtrl.InsertItem(str, item);
	
	for(i = 0; i < md.subDescriptorUIDs.num; i++)
	{
		str.Format(L"Sub Descriptor UID %d = ", i);
		for(j = 0; j < md.subDescriptorUIDs.size; j++)
		{
			temp.Format(L"%02x ", md.subDescriptorUIDs.ppItem[i][j]);		
			str += temp;
		}		
		m_treeCtrl.InsertItem(str, item);
	}	
}

void CMXFInspectorDlg::DumpMXFGenericPictureEssenceDescriptor(GenericPictureEssenceDescriptor& gped, HTREEITEM root)
{
	unsigned int i;
	CString str, temp;
	HTREEITEM item = 0;

	DumpMXFFileDescriptor(gped.fileDescriptor, root);

	if(gped.bSignalStandard)
	{
		switch(gped.signalStandard)
		{
		case 0x00: str = L"Signal Standard = 0x00 (No Specific Underlying Standard)"; break;
		case 0x01: str = L"Signal Standard = 0x01 (ITU-R BT.601 and BT.656, also SMPTE 125M (525 and 625 line interlaced))"; break;
		case 0x02: str = L"Signal Standard = 0x02 (ITU-R BT.1358, also SMPTE 293M (525 and 625 line progressive))"; break;
		case 0x03: str = L"Signal Standard = 0x03 (SMPTE 347M (540 Mbps mappings))"; break;
		case 0x04: str = L"Signal Standard = 0x04 (SMPTE 274M (1125 line))"; break;
		case 0x05: str = L"Signal Standard = 0x05 (SMPTE 296M (750 line progressive))"; break;
		case 0x06: str = L"Signal Standard = 0x06 (SMPTE 349M (1485 Mbps mappings))"; break;
		default:
			str.Format(L"Signal Standard = %d (desconhecido)", gped.signalStandard); break;
		}				
	}
	else
		str = L"Signal Standard = (optional and not specified)";

	m_treeCtrl.InsertItem(str, root);
	
	switch(gped.frameLayout)
	{
		case 0: str =  L"Frame Layout = 0 (Full Frame)"; break;
		case 1: str =  L"Frame Layout = 1 (Separate Fields)"; break;
		case 2: str =  L"Frame Layout = 2 (Single Field)"; break;
		case 3: str =  L"Frame Layout = 3 (Mixed Fields)"; break;
		case 4: str =  L"Frame Layout = 4 (Segmented Frame)"; break;
		default:
			str.Format(L"Frame Layout = %d (desconhecido)", gped.frameLayout);
	}	
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"StoredWidth = %d", gped.storedWidth);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"StoredHeight = %d", gped.storedHeight);
	m_treeCtrl.InsertItem(str, root);

	if(gped.bStoredF2Offset)
		str.Format(L"StoredF2Offset = %d", gped.storedF2Offset);
	else
		str.Format(L"StoredF2Offset = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(gped.bSampledWidth)
		str.Format(L"SampledWidth = %d", gped.sampledWidth);
	else
		str.Format(L"SampledWidth = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(gped.bSampledHeight)
		str.Format(L"SampledHeight = %d", gped.sampledHeight);
	else
		str.Format(L"SampledHeight = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(gped.bSampledXOffset)
		str.Format(L"SampledXOffset = %d", gped.sampledXOffset);
	else
		str.Format(L"SampledXOffset = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(gped.bSampledYOffset)
		str.Format(L"SampledYOffset = %d", gped.sampledYOffset);
	else
		str.Format(L"SampledYOffset = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(gped.bDisplayHeight)
		str.Format(L"DisplayHeight = %d", gped.displayHeight);
	else
		str.Format(L"DisplayHeight = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(gped.bDisplayWidth)
		str.Format(L"DisplayWidth = %d", gped.displayWidth);
	else
		str.Format(L"DisplayWidth = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(gped.bDisplayXOffset)
		str.Format(L"DisplayXOffset = %d", gped.displayXOffset);
	else
		str.Format(L"DisplayXOffset = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(gped.bDisplayYOffset)
		str.Format(L"DisplayYOffset = %d", gped.displayYOffset);
	else
		str.Format(L"DisplayYOffset = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(gped.bDisplayF2Offset)
		str.Format(L"DisplayF2Offset = %d", gped.displayF2Offset);
	else
		str.Format(L"DisplayF2Offset = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"AspectRatio = %d/%d", gped.aspectRatio.num, gped.aspectRatio.den);
	m_treeCtrl.InsertItem(str, root);

	if(gped.bActiveFormatDescriptor)
		str.Format(L"ActiveFormatDescriptor = %d", gped.activeFormatDescriptor);
	else
		str.Format(L"ActiveFormatDescriptor = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"VideoLineMap = {%d, %d}", gped.videoLineMap[0], gped.videoLineMap[1]);
	m_treeCtrl.InsertItem(str, root);

	if(gped.bAlphaTransparency)
		str.Format(L"AlphaTransparency = %d", gped.alphaTransparency);
	else
		str.Format(L"AlphaTransparency = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"CaptureGamma = ");
	if(gped.bCaptureGamma)
	{
		for(i = 0; i < 16; i++)
		{
			temp.Format(L"%02x ", gped.captureGamma[i]);
			str += temp;
		}
		item = m_treeCtrl.InsertItem(str, root);

		str.Format(L" (ver SMPTE 268, tabela 5)");
		m_treeCtrl.InsertItem(str, item);
	}
	else
	{
		str += "(optional and not specified)";
		m_treeCtrl.InsertItem(str, root);
	}

	if(gped.bImageAlignmentOffset)
		str.Format(L"ImageAlignmentOffset = %d", gped.imageAlignmentOffset);
	else
		str.Format(L"ImageAlignmentOffset = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(gped.bImageStartOffset)
		str.Format(L"ImageStartOffset = %d", gped.imageStartOffset);
	else
		str.Format(L"ImageStartOffset = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(gped.bImageEndOffset)
		str.Format(L"ImageEndOffset = %d", gped.imageEndOffset);
	else
		str.Format(L"ImageEndOffset = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(gped.bFieldDominance)
		str.Format(L"FieldDominance = %d", gped.fieldDominance);
	else
		str.Format(L"FieldDominance = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"PictureEssenceCoding = ");
	if(gped.bPictureEssenceCoding)
	{		
		for(i = 0; i < 16; i++)
		{
			temp.Format(L"%02x ", gped.pictureEssenceCoding[i]);
			str += temp;
		}
	}
	else
		str += L" (encoder optional and not specified)";
	m_treeCtrl.InsertItem(str, root);

}

void CMXFInspectorDlg::DumpMXFCDCIPictureEssenceDescriptor(CDCIPictureEssenceDescriptor& cped, HTREEITEM root)
{	
	HTREEITEM item = 0;
	CString str, temp;

	DumpMXFGenericPictureEssenceDescriptor(cped.genericPictureEssenceDescriptor, root);

	str.Format(L"ComponentDepth = %d", cped.componentDepth);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"HorizontalSubsampling = %d", cped.horizontalSubsampling);
	m_treeCtrl.InsertItem(str, root);

	if(cped.bVerticalSubsampling)
		str.Format(L"VerticalSubsampling = %d", cped.verticalSubsampling);
	else
		str.Format(L"VerticalSubsampling = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(cped.bColorSiting)
	{
		switch(cped.colorSiting)
		{
		case 0: str =  L"ColorSiting = 0 (coSiting)"; break;
		case 1: str =  L"ColorSiting = 1 (mid-point)"; break;
		case 2: str =  L"ColorSiting = 2 (threeTap)"; break;
		case 3: str =  L"ColorSiting = 3 (QuinCunx)"; break;
		case 4: str =  L"ColorSiting = 4 (Rec601)"; break;
		default: str.Format(L"ColorSiting = %d (Unknown)", cped.colorSiting); break;
		}		
	}
	else
		str.Format(L"ColorSiting = (optional and not specified)");	
	m_treeCtrl.InsertItem(str, root);

	if(cped.bReversedByteOrder)
		str.Format(L"ReversedByteOrder = %d", cped.reversedByteOrder);
	else
		str.Format(L"ReversedByteOrder = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(cped.bPaddingBits)
		str.Format(L"PaddingBits = %d", cped.paddingBits);
	else
		str.Format(L"PaddingBits = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(cped.bAlphaSampleDepth)
		str.Format(L"AlphaSampleDepth = %d", cped.alphaSampleDepth);
	else
		str.Format(L"AlphaSampleDepth = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(cped.bBlackRefLevel)
		str.Format(L"BlackRefLevel = %d", cped.blackRefLevel);
	else
		str.Format(L"BlackRefLevel = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(cped.bWhiteRefLevel)
		str.Format(L"WhiteRefLevel = %d", cped.whiteRefLevel);
	else
		str.Format(L"WhiteRefLevel = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(cped.bColorRange)
		str.Format(L"ColorRange = %d", cped.colorRange);
	else
		str.Format(L"ColorRange = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);
}

void CMXFInspectorDlg::DumpMXFGenericSoundEssenceDescriptor(GenericSoundEssenceDescriptor& gsed, HTREEITEM root)
{
	unsigned int i;	
	CString str, temp;

	DumpMXFFileDescriptor(gsed.fileDescriptor, root);

	str.Format(L"AudioSamplingRate = %d/%d", gsed.audioSamplingRate.num, gsed.audioSamplingRate.den);
	m_treeCtrl.InsertItem(str, root);

	if(gsed.bLockedUnlocked)
		str.Format(L"Locked/Unlocked = %d", gsed.lockedUnlocked);
	else
		str.Format(L"Locked/Unlocked = (encoder optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(gsed.bAudioRefLevel)
		str.Format(L"AudioRefLevel = %d", gsed.audioRefLevel);
	else
		str.Format(L"AudioRefLevel = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	if(gsed.bElectroSpatialFormulation)
	{
		switch(gsed.electroSpatialFormulation)
		{
		case 0: str = L"Electro Spatial Formulation = 0 (two-channel mode default)"; break;
		case 1: str = L"Electro Spatial Formulation = 1 (two-channel mode)"; break;
		case 2: str = L"Electro Spatial Formulation = 2 (single channel mode)"; break;
		case 3: str = L"Electro Spatial Formulation = 3 (primary/secondary mode)"; break;
		case 4: str = L"Electro Spatial Formulation = 4 (stereophonic mode)"; break;
		case 7: str = L"Electro Spatial Formulation = 7 (single channel, double frequency mode carried on 2 sub-frames)"; break;
		case 8: str = L"Electro Spatial Formulation = 8 (stereo left channel, double frequency mode carried on 2 sub-frames)"; break;
		case 9: str = L"Electro Spatial Formulation = 9 (stereo right channel, double frequency mode carried on 2 sub-frames)"; break;
		case 15: str = L"Electro Spatial Formulation = 15 (multi-channel mode default (>2 channels))"; break;
		default:
			str.Format(L"Electro Spatial Formulation = %d (undefined)", gsed.electroSpatialFormulation);
		}		
	}
	else
		str.Format(L"Electro Spatial Formulation = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"ChannelCount = %d", gsed.channelCount);
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"QuantizationBits = %d", gsed.quantizationBits);
	m_treeCtrl.InsertItem(str, root);

	if(gsed.bDialNorm)
		str.Format(L"DialNorm = %d", gsed.dialNorm);
	else
		str.Format(L"DialNorm = (optional and not specified)");
	m_treeCtrl.InsertItem(str, root);

	str.Format(L"SoundEssenceCompression = ");
	if(gsed.bSoundEssenceCompression)
	{		
		for(i = 0; i < 16; i++)
		{
			temp.Format(L"%02x ", gsed.soundEssenceCompression[i]);
			str += temp;
		}
	}
	else
		str += L"(encoder optional and not specified)";
	
	m_treeCtrl.InsertItem(str, root);
}

void CMXFInspectorDlg::OnGoto()
{
	if( m_fnameMXF.IsEmpty() )
		return;

	// Go to offset		
	CGoToDlg dlg;
	if( dlg.DoModal() == IDOK )
	{
		unsigned __int64 go = dlg.GoTo();	

		unsigned int i;
		
		for(i = 0; i < m_list.size(); i++)
		{
			if( go <= m_list[i].offset ) 
				break;
		}

		FindEntry(m_list[i].offset);
	}
}

void CMXFInspectorDlg::OnSave()
{
	if( m_fnameMXF.IsEmpty() )
		return;

	// Save dump
	CFileDialog dlg(FALSE, L"txt", 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, L"Log File (*.txt)|*.txt||", this, 0);			

	if( dlg.DoModal() == IDOK )
	{
		CString saveName = dlg.GetPathName();

		MXFLogger logger(m_fnameMXF.GetString());
		logger.Save(saveName.GetString());
	}
}

void CMXFInspectorDlg::FindEntry(unsigned __int64 offset)
{
	CString str;	
	str.Format(L"<%016I64x", offset);

	LVFINDINFO info = {0};
	int nIndex;

	info.flags = LVFI_STRING|LVFI_WRAP|LVFI_PARTIAL;
	info.psz = str;

	if( (nIndex=m_listCtrl.FindItem(&info)) != -1 )
	{
		m_listCtrl.SetFocus();
		m_listCtrl.EnsureVisible(nIndex, FALSE);
		m_listCtrl.SetItemState(nIndex, LVIS_SELECTED, LVIS_SELECTED);
		DumpEntry(offset);
	}
}
void CMXFInspectorDlg::OnFileExportdump()
{
	OnSave();	
}

void CMXFInspectorDlg::OnFileExportvideo()
{
	// Save dump
	CFileDialog dlg(FALSE, L"mpg", 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, L"Elementary stream (*.mpg)|*.mpg||", this, 0);			

	if( dlg.DoModal() == IDOK )
	{
		CString saveName = dlg.GetPathName();	

		unsigned char keyValue[16];
		unsigned char ch;
		DWORD size, szSize, nRead;	
		HANDLE hMXF;
		BOOL bSeek;

		hMXF = CreateFile(m_fnameMXF, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if(hMXF == INVALID_HANDLE_VALUE)
		{
			AfxMessageBox(L"Erro ao abrir arquivo MXF\n");
			return;
		}

		unsigned __int64 filePos;
		
		while(true)
		{
			filePos = GetFilePosition(hMXF);

			ReadFile(hMXF, keyValue, 16, &nRead, NULL);

			if(nRead < 16)
			{		
				break;
			}		

			ReadFile(hMXF, &ch, 1, &nRead, NULL);
			if( ch & 0x80 )
			{
				szSize = ch&0x0f;
				size = (DWORD)GetNumber(hMXF, szSize);
			}
			else
			{
				size = ch;
			}

			bSeek = TRUE;

			if(!memcmp(keyValue, cppictureItemKey, sizeof(cppictureItemKey)) ||
				!memcmp(keyValue, gcpictureItemKey, sizeof(gcpictureItemKey)) ||
				!memcmp(keyValue, gccompoundItemKey, sizeof(gccompoundItemKey)) )
			{			
				LPBYTE pBuffer = new BYTE[size];
				ReadFile(hMXF, pBuffer, size, &nRead, NULL);

				FILE* fp;
				fp = _wfopen(saveName.GetString(), L"ab");
				fwrite(pBuffer, sizeof(BYTE), size, fp);
				fclose(fp);

				delete []pBuffer;

				bSeek = false;
			}

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

		if( hMXF )
			CloseHandle(hMXF);

	}
}

void CMXFInspectorDlg::OnFileExportaudio()
{
	// Save dump
	CFileDialog dlg(FALSE, L"mpg", 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, L"Elementary stream (*.mpg)|*.mpg||", this, 0);			

	if( dlg.DoModal() == IDOK )
	{
		CString saveName = dlg.GetPathName();	

		unsigned char keyValue[16];
		unsigned char ch;
		DWORD size, szSize, nRead;	
		HANDLE hMXF;
		BOOL bSeek;

		hMXF = CreateFile(m_fnameMXF, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if(hMXF == INVALID_HANDLE_VALUE)
		{
			AfxMessageBox(L"Erro ao abrir arquivo MXF\n");
			return;
		}

		unsigned __int64 filePos;

		FILE* fp;
		fp = _wfopen(saveName.GetString(), L"wb");

		int c = 0;

		while(true)
		{
			filePos = GetFilePosition(hMXF);

			ReadFile(hMXF, keyValue, 16, &nRead, NULL);

			if(nRead < 16)
				break;
			
			ReadFile(hMXF, &ch, 1, &nRead, NULL);
			if( ch & 0x80 )
			{
				szSize = ch&0x0f;
				size = (DWORD)GetNumber(hMXF, szSize);
			}
			else
			{
				size = ch;
			}
					
			bSeek = TRUE;

			if(!memcmp(keyValue, gcsoundItemKey, sizeof(gcsoundItemKey)) ||
				!memcmp(keyValue, cpsoundItemKey, sizeof(cpsoundItemKey)) )
			{			
				LPBYTE pBuffer = new BYTE[size];
				ReadFile(hMXF, pBuffer, size, &nRead, NULL);
				
				c++;

				size = nRead;
				if( c == 2 )
					fwrite(pBuffer, sizeof(BYTE), size, fp);

				if( c == 4 )
					c = 0;
				/*if( m_audioType == PCM_AUDIO )
				{
					size = nRead;
					fwrite(pBuffer, sizeof(BYTE), size, fp);
				}
				else
				{
					CAESDecoder aes;					
					unsigned int pcmSize = 1602 * 8 * 2;
					LPBYTE pcmBuf = new BYTE[pcmSize];
					unsigned int nChannels = 0, samplesCount = 0;					 
					aes.DecodeAES3Frame(pBuffer, nRead, pcmBuf, &pcmSize, &nChannels, &samplesCount);
					fwrite(pcmBuf, sizeof(BYTE), pcmSize, fp);
					delete []pcmBuf;
				}*/

				delete []pBuffer;

				bSeek = false;
			}

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

		fclose(fp);

		if( hMXF )
			CloseHandle(hMXF);


	}
}

void CMXFInspectorDlg::OnFilterPack()
{
	m_listCtrl.DeleteAllItems();
	std::vector<MyItemList>::iterator it;

	for(it = m_list.begin(); it != m_list.end(); it++)
	{
		if( IsPack((*it).node) )
		{
			InsertEntry((*it).offset, 
						(*it).node,	
						(*it).key,	
						(*it).length,	
						(*it).info, false);
		}
	}

	CMenu* menu = GetMenu();
	CMenu* submenu = menu->GetSubMenu(1);
	if( submenu )
	{
		for(int j = 0; j < 7; j++)
			submenu->CheckMenuItem(j, MF_UNCHECKED | MF_BYPOSITION);		

		submenu->CheckMenuItem(0, MF_CHECKED | MF_BYPOSITION);		
	}
}

void CMXFInspectorDlg::OnFilterPicture()
{
	m_listCtrl.DeleteAllItems();
	std::vector<MyItemList>::iterator it;

	for(it = m_list.begin(); it != m_list.end(); it++)
	{
		if( IsPicture((*it).node) )
		{
			InsertEntry((*it).offset, 
						(*it).node,	
						(*it).key,	
						(*it).length,	
						(*it).info, false);
		}
	}

	CMenu* menu = GetMenu();
	CMenu* submenu = menu->GetSubMenu(1);
	if( submenu )
	{
		for(int j = 0; j < 7; j++)
			submenu->CheckMenuItem(j, MF_UNCHECKED | MF_BYPOSITION);		

		submenu->CheckMenuItem(1, MF_CHECKED | MF_BYPOSITION);		
	}
}

void CMXFInspectorDlg::OnFilterSound()
{
	m_listCtrl.DeleteAllItems();
	std::vector<MyItemList>::iterator it;

	for(it = m_list.begin(); it != m_list.end(); it++)
	{
		if( IsSound((*it).node) )
		{
			InsertEntry((*it).offset, 
						(*it).node,	
						(*it).key,	
						(*it).length,	
						(*it).info, false);
		}
	}

	CMenu* menu = GetMenu();
	CMenu* submenu = menu->GetSubMenu(1);
	if( submenu )
	{
		for(int j = 0; j < 6; j++)
			submenu->CheckMenuItem(j, MF_UNCHECKED | MF_BYPOSITION);		

		submenu->CheckMenuItem(2, MF_CHECKED | MF_BYPOSITION);		
	}
}

void CMXFInspectorDlg::OnFilterMetadata()
{
	m_listCtrl.DeleteAllItems();
	std::vector<MyItemList>::iterator it;

	for(it = m_list.begin(); it != m_list.end(); it++)
	{
		if( IsMetadata((*it).node) )
		{
			InsertEntry((*it).offset, 
						(*it).node,	
						(*it).key,	
						(*it).length,	
						(*it).info, false);
		}
	}

	CMenu* menu = GetMenu();
	CMenu* submenu = menu->GetSubMenu(1);
	if( submenu )
	{
		for(int j = 0; j < 7; j++)
			submenu->CheckMenuItem(j, MF_UNCHECKED | MF_BYPOSITION);		

		submenu->CheckMenuItem(3, MF_CHECKED | MF_BYPOSITION);		
	}
}

void CMXFInspectorDlg::OnFilterIndextable()
{
	m_listCtrl.DeleteAllItems();
	std::vector<MyItemList>::iterator it;

	for(it = m_list.begin(); it != m_list.end(); it++)
	{
		if( IsIndexTable((*it).node) )
		{
			InsertEntry((*it).offset, 
						(*it).node,	
						(*it).key,	
						(*it).length,	
						(*it).info, false);
		}
	}

	CMenu* menu = GetMenu();
	CMenu* submenu = menu->GetSubMenu(1);
	if( submenu )
	{
		for(int j = 0; j < 7; j++)
			submenu->CheckMenuItem(j, MF_UNCHECKED | MF_BYPOSITION);		

		submenu->CheckMenuItem(4, MF_CHECKED | MF_BYPOSITION);		
	}
}

void CMXFInspectorDlg::OnFilterShowAll()
{
	m_listCtrl.DeleteAllItems();
	std::vector<MyItemList>::iterator it;

	for(it = m_list.begin(); it != m_list.end(); it++)
	{		
		InsertEntry((*it).offset, 
					(*it).node,	
					(*it).key,	
					(*it).length,	
					(*it).info, false);
	}

	CMenu* menu = GetMenu();
	CMenu* submenu = menu->GetSubMenu(1);
	if( submenu )
	{
		for(int j = 0; j < 7; j++)
			submenu->CheckMenuItem(j, MF_UNCHECKED | MF_BYPOSITION);		

		submenu->CheckMenuItem(6, MF_CHECKED | MF_BYPOSITION);		
	}
}


BOOL CheckHeaderOffset(CString fnameMXF)
{
	unsigned char keyValue[16];
	unsigned char ch;
	DWORD size, szSize, nRead;	
	BOOL bSeek;
	HANDLE hMXF = 0;
	
	unsigned __int64 header_byte_count = 0;	
	
	hMXF = CreateFile(fnameMXF, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hMXF == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(L"Erro ao abrir arquivo MXF\n");
		return FALSE;
	}
	
	unsigned __int64 filePos = 0;
	
	while(true)
	{
		filePos = GetFilePosition(hMXF);
		ReadFile(hMXF, keyValue, 16, &nRead, NULL);
		if(nRead < 16)
			break; // EOF


		ReadFile(hMXF, &ch, 1, &nRead, NULL);
		if( ch & 0x80 )
		{
			szSize = ch&0x0f;
			size = (DWORD)GetNumber(hMXF, szSize);
		}
		else
		{
			size = ch;
		}
		
		bSeek = TRUE;

		if(!memcmp(keyValue, headerKey, sizeof(headerKey)))
		{			
			ASSERT(keyValue[15] == 0x00);			

			PartitionPack pack;
			pack.Read(hMXF, size);
			bSeek = FALSE;
			
			header_byte_count = pack.headerByteCount;
		}

		if(!memcmp(keyValue, bodyKey, sizeof(bodyKey)))
		{
			ASSERT(keyValue[15] == 0x00);

			PartitionPack pack;
			pack.Read(hMXF, size);
			bSeek = FALSE;

			ASSERT( header_byte_count == (filePos - pack.kagSize) );
			break;
		}

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

	if( hMXF )
		CloseHandle(hMXF);

	return TRUE;
}

BOOL CheckFooterPartition(CString fnameMXF)
{
	unsigned char keyValue[16];
	unsigned char ch;
	DWORD size, szSize, nRead;	
	BOOL bSeek;
	HANDLE hMXF = 0;
	
	unsigned __int64 footer_partition = 0;
	
	hMXF = CreateFile(fnameMXF, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hMXF == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(L"Erro ao abrir arquivo MXF\n");
		return FALSE;
	}

	unsigned __int64 filePos = 0;
	
	while(true)
	{
		filePos = GetFilePosition(hMXF);
		ReadFile(hMXF, keyValue, 16, &nRead, NULL);
		if(nRead < 16)
			break; // EOF


		ReadFile(hMXF, &ch, 1, &nRead, NULL);
		if( ch & 0x80 )
		{
			szSize = ch&0x0f;
			size = (DWORD)GetNumber(hMXF, szSize);
		}
		else
		{
			size = ch;
		}
				
		bSeek = TRUE;

		if(!memcmp(keyValue, headerKey, sizeof(headerKey)))
		{			
			ASSERT(keyValue[15] == 0x00);			

			PartitionPack pack;
			pack.Read(hMXF, size);
			bSeek = FALSE;
			
			footer_partition = pack.footerPartition;
		}

		if(!memcmp(keyValue, footerKey, sizeof(footerKey)))
		{
			ASSERT(keyValue[15] == 0x00);	
			ASSERT( footer_partition == filePos );
			break;
		}
		
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

	if( hMXF )
		CloseHandle(hMXF);

	return TRUE;
}
