//	This file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#include "FDAviInfo.h"
#include "..\emule.h"
#include "..\otherfunctions.h"
#include <mmsystem.h>
#include <avifmt.h>

IMPLEMENT_DYNCREATE(CFDAviInfo, CPropertyPage)

CFDAviInfo::CFDAviInfo() : CPropertyPage(CFDAviInfo::IDD)
, m_bARoundBitrate(FALSE)
{
	m_pFile = NULL;
}

CFDAviInfo::~CFDAviInfo()
{
}

void CFDAviInfo::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WIDTH_VAL, m_ctrlWidth);
	DDX_Control(pDX, IDC_VIDEOCODEC_VAL, m_ctrlVideoCodec);
	DDX_Control(pDX, IDC_VIDEOBITRATE_VAL, m_ctrlVideoBitrate);
	DDX_Control(pDX, IDC_SAMPLERATE_VAL, m_ctrlSamplerate);
	DDX_Control(pDX, IDC_HEIGHT_VAL, m_ctrlHeight);
	DDX_Control(pDX, IDC_FPS_VAL, m_ctrlFPS);
	DDX_Control(pDX, IDC_CHANNEL_VAL, m_ctrlChannel);
	DDX_Control(pDX, IDC_AUDIOCODEC_VAL, m_ctrlAudioCodec);
	DDX_Control(pDX, IDC_AUDIOBITRATE_VAL, m_ctrlAudioBitrate);
	DDX_Control(pDX, IDC_VIDEO_LBL, m_ctrlVideoLbl);
	DDX_Control(pDX, IDC_PICTURE_LBL, m_ctrlPictureLbl);
	DDX_Control(pDX, IDC_AUDIO_LBL, m_ctrlAudioLbl);
	DDX_Check(pDX, IDC_ROUNDBITRATE_BTN, m_bARoundBitrate);
	DDX_Control(pDX, IDC_FILESIZE_VAL, m_ctrlFilesize);
	DDX_Control(pDX, IDC_FILELENGTH_VAL, m_ctrlFilelength);
}


BEGIN_MESSAGE_MAP(CFDAviInfo, CPropertyPage)
	ON_BN_CLICKED(IDC_ROUNDBITRATE_BTN, OnBnClickedRoundbitrateBtn)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFDAviInfo message handlers

BOOL CFDAviInfo::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_bARoundBitrate = TRUE;
	m_dwAbitrate = 0;
	CPropertyPage::UpdateData(FALSE);
	Localize();
	Update();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

static bool AVIReadChunkHdr(CFile *file, uint32 *pdwFCC, uint32 *pdwLen)
{
	uint32		dwTmp[2];
	bool		bResult = (file->Read(dwTmp, 8) == 8);

	if (bResult)
	{
		*pdwFCC = dwTmp[0];
		*pdwLen = dwTmp[1];
	}

	return bResult;
}

void CFDAviInfo::Update()
{
	EMULE_TRY

	if ((m_pFile == NULL) || !::IsWindow(GetSafeHwnd()))
		return;

	CFile		file;

	if ( file.Open(m_pFile->GetFilePath(), CFile::modeRead | CFile::shareDenyNone |
		CFile::typeBinary | CFile::osSequentialScan) )
	{
		bool	bAVIHdr = false, bVideoFmt = false, bAudioFmt = false;
		int		iVideoStrm = -1, iAudioStrm = -1, iCurrStrm;
		uint32	dwTmp, dwAVISize = 0;
		uint64	qwAVISize;
		AVIStreamHeader		strmAVIHdr[2];
		MainAVIHeader		mainAVIHdr;
		BITMAPINFOHEADER	vidFormat;
		WAVEFORMATEX		audFormat;

		try
		{
			uint32	dwFCC, dwLen, dwLeft, dwStrmIdx = 0;

		//	Read file header and check 'RIFF' signature
			if (AVIReadChunkHdr(&file, &dwFCC, &dwAVISize) && (dwFCC == FOURCC_RIFF))
			{
			//	Some AVI can be > 4GB, but standard file header field is 32bit;
			//	So we use actual file size for large files and still header field for small,
			//	as it is more precise in some cases
				if (m_pFile->GetFileSize() > 0xFFFFFFFFui64)
					qwAVISize = m_pFile->GetFileSize();
				else
					qwAVISize = static_cast<uint64>(dwAVISize);
			//	Check AVI signature
				if ((file.Read(&dwFCC, 4) == 4) && (dwFCC == formtypeAVI))
				{
				//	Dispatch chunks
					while (AVIReadChunkHdr(&file, &dwFCC, &dwLen))
					{
					//	Check for potentially incorrect FCC (nonprintable characters)
						if ( ((dwTmp = dwFCC & 0xFF) < 0x20) || (dwTmp > 0x7E) ||
							((dwTmp = (dwFCC >>  8) & 0xFF) < 0x20) || (dwTmp > 0x7E) ||
							((dwTmp = (dwFCC >> 16) & 0xFF) < 0x20) || (dwTmp > 0x7E) ||
							((dwTmp = (dwFCC >> 24)) < 0x20) || (dwTmp > 0x7E) )
						{
							goto while_end;
						}
						switch (dwFCC)
						{
							case FOURCC_LIST:
							//	Read sub-chunk name and check LIST chunk minimal size
							//	(excluding main header as LIST chunk size doesn't required here)
								if ((file.Read(&dwFCC, 4) != 4) || ((dwLen < 4) && (dwFCC != listtypeAVIHEADER)))
									goto while_end;

							//	Reduce the length by just read sub-chunk name size
								dwLen -= 4;
								switch (dwFCC)
								{
									case listtypeAVIMOVIE:	// data section, header must have been before
										goto while_end;
									case listtypeAVIHEADER:	// 'hdrl' main header
										dwLen = 0;	// process sub-chunks in the next iteration
										break;
									case listtypeSTREAMHEADER:
										dwLeft = dwLen;
									//	Stream header type as well as protection that stream header was found
										iCurrStrm = 0;
										while((dwLeft >= 8) && AVIReadChunkHdr(&file, &dwFCC, &dwLen))
										{
											dwLeft -= 8;
											if (dwLen > dwLeft)
												goto while_end;
											dwTmp = dwLeft;
											dwLeft -= dwLen + (dwLen & 1);
											if (dwLeft > dwTmp)
												dwLeft = 0;

											if (dwFCC == ckidSTREAMHEADER)
											{
												if (dwLen < sizeof(strmAVIHdr[0]))
												{
													memset(&strmAVIHdr[dwStrmIdx], 0, sizeof(strmAVIHdr[0]));
													if (file.Read(&strmAVIHdr[dwStrmIdx], dwLen) != dwLen)
														goto while_end;
													if ((dwLen & 1) != 0)
														file.Seek(1, CFile::current);
													dwLen = 0;
												}
												else
												{
													if (file.Read(&strmAVIHdr[dwStrmIdx], sizeof(strmAVIHdr[0])) != sizeof(strmAVIHdr[0]))
														goto while_end;
													dwLen -= sizeof(strmAVIHdr[0]);
												}
											//	NB: AVI Type-1 is not supported
												if ((strmAVIHdr[dwStrmIdx].fccType == streamtypeVIDEO) && (iVideoStrm < 0))
													iCurrStrm = dwStrmIdx + 1;
												else if ((strmAVIHdr[dwStrmIdx].fccType == streamtypeAUDIO) && (iAudioStrm < 0))
													iCurrStrm = -static_cast<int>(dwStrmIdx + 1);
											}
										//	Stream header tag should have been before
											else if ((dwFCC == ckidSTREAMFORMAT) && (iCurrStrm != 0))
											{
												void	*pPtr;

												dwTmp = (iCurrStrm > 0) ?
													(pPtr = &vidFormat, sizeof(vidFormat)) : (pPtr = &audFormat, sizeof(audFormat));
											//	Data reading
												if (dwLen < dwTmp)
												{
													memset(pPtr, 0, dwTmp);
													if (file.Read(pPtr, dwLen) != dwLen)
														goto while_end;
													if ((dwLen & 1) != 0)
														file.Seek(1, CFile::current);
													dwLen = 0;
												}
												else
												{
													if (file.Read(pPtr, dwTmp) != dwTmp)
														goto while_end;
													if ((dwLen -= dwTmp) != 0)
														file.Seek(dwLen + (dwLen & 1), CFile::current);
												}
											//	Mark the corresponding structure as filled
												if (iCurrStrm > 0)
													bVideoFmt = true;
												else
													bAudioFmt = true;
											//	One stream specification per LIST section
												break;
											}
											if (dwLen != 0)
												file.Seek(dwLen + (dwLen & 1), CFile::current);
										}
										if (iCurrStrm != 0)
										{
											if (iCurrStrm > 0)
												iVideoStrm = dwStrmIdx;
											else
												iAudioStrm = dwStrmIdx;
										//	Nothing to do here anymore if video and audio streams were parsed
											if (++dwStrmIdx >= 2)
												goto while_end;
										}

										dwLen = dwLeft;
										break;
								}
								break;

							case ckidAVIMAINHDR:	// 'avih' main AVI header
							//	Consider only the first one
								if (!bAVIHdr)
								{
									dwTmp = (dwLen > sizeof(mainAVIHdr)) ? sizeof(mainAVIHdr) : dwLen;
									memset(&mainAVIHdr, 0, sizeof(mainAVIHdr));
									if (file.Read(&mainAVIHdr, dwTmp) != dwTmp)
										goto while_end;

									dwLen -= dwTmp;
									bAVIHdr = true;
								}
								break;

							case ckidAVINEWINDEX:	// 'idx1' chunk, it's the end of the file -- finish search
								goto while_end;
						}
						if (dwLen != 0)
							file.Seek(dwLen + (dwLen & 1), CFile::current);
					}
while_end: ;
				}
			}
		}
		catch(CFileException *error)
		{
			OUTPUT_DEBUG_TRACE();
			error->Delete();
		}
		file.Close();

	//	AVI header was dispatched
		if (bAVIHdr)
		{
			CString	strBuffer;
			uint32	dwLengthInSec = 0;

			m_ctrlFilesize.SetWindowText(CastItoXBytes(qwAVISize));

		//	Protect from division by zero
			if (mainAVIHdr.dwMicroSecPerFrame != 0)
			{
			//	FPS
				double dFps = 1000000.0 / (double)mainAVIHdr.dwMicroSecPerFrame;

				strBuffer.Format(_T("%.3f"), dFps);
				m_ctrlFPS.SetWindowText(strBuffer);

			//	Duration
				dwLengthInSec = static_cast<uint32>(mainAVIHdr.dwTotalFrames / dFps);
				m_ctrlFilelength.SetWindowText(CastSecondsToHM(dwLengthInSec));
			}

		//	Video width
			strBuffer.Format(_T("%u"), mainAVIHdr.dwWidth);
			m_ctrlWidth.SetWindowText(strBuffer);
		//	Video height
			strBuffer.Format(_T("%u"), mainAVIHdr.dwHeight);
			m_ctrlHeight.SetWindowText(strBuffer);

		//	Audio stream with audio format was found
			if ((iAudioStrm >= 0) && bAudioFmt)
			{
			//	Audio codec
				dwTmp = audFormat.wFormatTag;
				switch (dwTmp)
				{
					case 0:
					case 1:
						m_ctrlAudioCodec.SetWindowText(_T("PCM"));
						break;
					case 2:
						m_ctrlAudioCodec.SetWindowText(_T("ADPCM"));
						break;
					case 353:
						m_ctrlAudioCodec.SetWindowText(_T("DivX;-) Audio"));
						break;
					case 85:
						m_ctrlAudioCodec.SetWindowText(_T("MPEG Layer 3"));
						break;
					case 8192:
						m_ctrlAudioCodec.SetWindowText(_T("AC3-Digital"));
						break;
					case 0x674F:
						m_ctrlAudioCodec.SetWindowText(_T("Ogg Vorbis (mode 1)"));
						break;
					case 0x6750:
						m_ctrlAudioCodec.SetWindowText(_T("Ogg Vorbis (mode 2)"));
						break;
					case 0x6751:
						m_ctrlAudioCodec.SetWindowText(_T("Ogg Vorbis (mode 3)"));
						break;
					case 0x676F:
						m_ctrlAudioCodec.SetWindowText(_T("Ogg Vorbis (mode 1+)"));
						break;
					case 0x6770:
						m_ctrlAudioCodec.SetWindowText(_T("Ogg Vorbis (mode 2+)"));
						break;
					case 0x6771:
						m_ctrlAudioCodec.SetWindowText(_T("Ogg Vorbis (mode 3+)"));
						break;
					default:
						GetResString(&strBuffer, IDS_UNKNOWN);
						strBuffer.AppendFormat(_T(" (%#x)"), dwTmp);
						m_ctrlAudioCodec.SetWindowText(strBuffer);
						break;
				}
			//	Audio channel
				dwTmp = audFormat.nChannels;
				switch (dwTmp)
				{
					case 1:
						m_ctrlChannel.SetWindowText(_T("1 (mono)"));
						break;
					case 2:
						m_ctrlChannel.SetWindowText(_T("2 (stereo)"));
						break;
					default:
						if ((dwTmp == 5) && (audFormat.wFormatTag == 8192))
							m_ctrlChannel.SetWindowText(_T("5.1 (surround)"));
						else
						{
							strBuffer.Format(_T("%u"), dwTmp);
							m_ctrlChannel.SetWindowText(strBuffer);
						}
						break;
				}
			//	Audio samplerate
				strBuffer.Format(_T("%u"), audFormat.nSamplesPerSec);
				m_ctrlSamplerate.SetWindowText(strBuffer);
			//	Audio bitrate
				m_dwAbitrate = audFormat.nAvgBytesPerSec;
				GetDlgItem(IDC_ROUNDBITRATE_BTN)->EnableWindow(true);
				OnBnClickedRoundbitrateBtn();
			}

		//	Video stream was found
			if (iVideoStrm >= 0)
			{
			//	Video codec
				if (!bVideoFmt || ((dwTmp = vidFormat.biCompression) != '05XD'))
					dwTmp = strmAVIHdr[iVideoStrm].fccHandler;
				if ((dwTmp == 'divx') || (dwTmp == 'DIVX'))
					m_ctrlVideoCodec.SetWindowText(_T("XviD"));
				else if (dwTmp == '05XD')
					m_ctrlVideoCodec.SetWindowText(_T("DivX 5"));
				else if ((dwTmp == 'xvid') || (dwTmp == 'XVID'))
					m_ctrlVideoCodec.SetWindowText(_T("DivX 4"));
				else if ((dwTmp == '3vid') || (dwTmp == '3VID'))
					m_ctrlVideoCodec.SetWindowText(_T("DivX 3 Low-Motion"));
				else if ((dwTmp == '4vid') || (dwTmp == '4VID'))
					m_ctrlVideoCodec.SetWindowText(_T("DivX 3 High-Motion"));
				else if ((dwTmp == '2vid') || (dwTmp == '2VID'))
					m_ctrlVideoCodec.SetWindowText(_T("MS MPEG4 v2"));
				else if (dwTmp == '34pm')
					m_ctrlVideoCodec.SetWindowText(_T("MS MPEG4 v3"));
				else
				{
					char	acTmp[5];

					*reinterpret_cast<uint32*>(acTmp) = dwTmp;
					acTmp[4] = '\0';
					strBuffer.Format(_T("%hs"), acTmp);
					m_ctrlVideoCodec.SetWindowText(strBuffer);
				}

			//	Protect from division by zero
				if (dwLengthInSec != 0)
				{
				//	Video bitrate (1 Kbit = 1000 bits)
					strBuffer.Format(_T("%u kbps"), ( (dwAVISize - mainAVIHdr.dwTotalFrames * 24) /
						dwLengthInSec - m_dwAbitrate ) / (1000 / 8));
					m_ctrlVideoBitrate.SetWindowText(strBuffer);
				}
			}
		}
	}

	EMULE_CATCH
}

void CFDAviInfo::OnBnClickedRoundbitrateBtn()
{
	EMULE_TRY

	if (m_dwAbitrate != 0)
	{
		CPropertyPage::UpdateData();

		uint32		dwKbit = m_dwAbitrate / (1000 / 8);	// 1 Kbit = 1000 bits

		if (m_bARoundBitrate)
		{
			if ((dwKbit >= 246) && (dwKbit <= 260))
			{
				m_ctrlAudioBitrate.SetWindowText(_T("256 kbps"));
			}
			else if ((dwKbit >= 216) && (dwKbit <= 228))
			{
				m_ctrlAudioBitrate.SetWindowText(_T("224 kbps"));
			}
			else if ((dwKbit >= 187) && (dwKbit <= 196))
			{
				m_ctrlAudioBitrate.SetWindowText(_T("192 kbps"));
			}
			else if ((dwKbit >= 156) && (dwKbit <= 164))
			{
				m_ctrlAudioBitrate.SetWindowText(_T("160 kbps"));
			}
			else if ((dwKbit >= 124) && (dwKbit <= 132))
			{
				m_ctrlAudioBitrate.SetWindowText(_T("128 kbps"));
			}
			else if ((dwKbit >= 108) && (dwKbit <= 116))
			{
				m_ctrlAudioBitrate.SetWindowText(_T("112 kbps"));
			}
			else if ((dwKbit >= 92) && (dwKbit <= 100))
			{
				m_ctrlAudioBitrate.SetWindowText(_T("96 kbps"));
			}
			else if ((dwKbit >= 60) && (dwKbit <= 68))
			{
				m_ctrlAudioBitrate.SetWindowText(_T("64 kbps"));
			}
			else
			{
				CString strBuffer;
				strBuffer.Format(_T("%u kbps"), dwKbit);
				m_ctrlAudioBitrate.SetWindowText(strBuffer);
			}
		}
		else
		{
			CString strBuffer;
			strBuffer.Format(_T("%u kbps"), dwKbit);
			m_ctrlAudioBitrate.SetWindowText(strBuffer);
		}
	}
	else
	{
		m_ctrlAudioBitrate.SetWindowText(_T("-"));
	}

	EMULE_CATCH
}

void CFDAviInfo::Localize()
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_PICTURE_LBL, IDS_FD_PICTURE },
		{ IDC_AUDIO_LBL, IDS_AUDIO },
		{ IDC_ROUNDBITRATE_BTN, IDS_ROUNDBITRATE },
		{ IDC_VIDEO_LBL, IDS_VIDEO },
		{ IDC_FILESIZE_LBL, IDS_FD_SIZE }
	};
	static const uint16 s_auResTbl2[][2] =
	{
		{ IDC_WIDTH_LBL, IDS_WIDTH },
		{ IDC_HEIGHT_LBL, IDS_HEIGHT },
		{ IDC_AUDIOCODEC_LBL, IDS_CODEC },
		{ IDC_AUDIOBITRATE_LBL, IDS_BITRATE },
		{ IDC_SAMPLERATE_LBL, IDS_SAMPLERATE },
		{ IDC_CHANNEL_LBL, IDS_CHANNELS },
		{ IDC_VIDEOCODEC_LBL, IDS_CODEC },
		{ IDC_VIDEOBITRATE_LBL, IDS_BITRATE },
		{ IDC_FPS_LBL, IDS_FPS },
		{ IDC_FILELENGTH_LBL, IDS_LENGTH }
	};

	if (GetSafeHwnd())
	{
		CString strBuffer;

		for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
		{
			GetResString(&strBuffer, static_cast<UINT>(s_auResTbl[i][1]));
			SetDlgItemText(s_auResTbl[i][0], strBuffer);
		}

		for (uint32 i = 0; i < ARRSIZE(s_auResTbl2); i++)
		{
			GetResString(&strBuffer, static_cast<UINT>(s_auResTbl2[i][1]));
			strBuffer += _T(":");
			SetDlgItemText(s_auResTbl2[i][0], strBuffer);
		}
	}
}
