//=============================================================================//
//
// Purpose: Completion functions for ConCommand callbacks.
//
//=============================================================================//

#include "core/stdafx.h"
#include "windows/id3dx.h"
#include "tier1/cvar.h"
#include "tier1/IConVar.h"
#ifndef DEDICATED
#include "engine/cl_rcon.h"
#endif // !DEDICATED
#include "engine/net.h"
#include "engine/net_chan.h"
#include "engine/sys_utils.h"
#include "engine/baseclient.h"
#include "rtech/rtech_game.h"
#include "rtech/rtech_utils.h"
#include "filesystem/basefilesystem.h"
#include "filesystem/filesystem.h"
#include "vpklib/packedstore.h"
#include "squirrel/sqvm.h"
#ifndef DEDICATED
#include "gameui/IBrowser.h"
#include "gameui/IConsole.h"
#endif // !DEDICATED
#include "public/include/bansystem.h"
#include "mathlib/crc32.h"
#include "vstdlib/completion.h"
#ifndef DEDICATED
#include "materialsystem/cmaterialglue.h"
#endif // !DEDICATED


#ifndef DEDICATED
/*
=====================
_CGameConsole_f_CompletionFunc
=====================
*/
void _CGameConsole_f_CompletionFunc(const CCommand& args)
{
	g_pIConsole->m_bActivate = !g_pIConsole->m_bActivate;
}

/*
=====================
_CCompanion_f_CompletionFunc
=====================
*/
void _CCompanion_f_CompletionFunc(const CCommand& args)
{
	g_pIBrowser->m_bActivate = !g_pIBrowser->m_bActivate;
}
#endif // !DEDICATED

/*
=====================
_Kick_f_CompletionFunc
=====================
*/
void _Kick_f_CompletionFunc(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		return;
	}

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		CBaseClient* pClient = g_pClient->GetClient(i);
		CNetChan* pNetChan = pClient->GetNetChan();
		if (!pClient || !pNetChan)
		{
			continue;
		}

		std::string svClientName = pNetChan->GetName(); // Get full name.

		if (svClientName.empty())
		{
			continue;
		}

		if (strcmp(args.Arg(1), svClientName.c_str()) != 0) // Our wanted name?
		{
			continue;
		}

		NET_DisconnectClient(pClient, i, "Kicked from Server", 0, 1);
	}
}

/*
=====================
_KickID_f_CompletionFunc
=====================
*/
void _KickID_f_CompletionFunc(const CCommand& args)
{
	if (args.ArgC() < 2) // Do we atleast have 2 arguments?
	{
		return;
	}

	try
	{
		bool bOnlyDigits = args.HasOnlyDigits(1);
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			CBaseClient* pClient = g_pClient->GetClient(i);
			CNetChan* pNetChan = pClient->GetNetChan();

			if (!pClient || !pNetChan)
			{
				continue;
			}

			std::string svIpAddress = pNetChan->GetAddress(); // If this stays null they modified the packet somehow.

			if (bOnlyDigits)
			{
				std::int64_t nTargetID = static_cast<std::int64_t>(std::stoll(args.Arg(1)));
				if (nTargetID > MAX_PLAYERS) // Is it a possible originID?
				{
					std::int64_t nOriginID = pClient->GetOriginID();
					if (nOriginID != nTargetID)
					{
						continue;
					}
				}
				else // If its not try by userID.
				{
					std::int64_t nClientID = static_cast<std::int64_t>(pClient->GetUserID() + 1); // Get userID + 1.
					if (nClientID != nTargetID)
					{
						continue;
					}
				}

				NET_DisconnectClient(pClient, i, "Kicked from Server", 0, 1);
			}
			else
			{
				if (std::string(args.Arg(1)).compare(svIpAddress) != NULL)
				{
					continue;
				}

				NET_DisconnectClient(pClient, i, "Kicked from Server", 0, 1);
			}
		}
	}
	catch (std::exception& e)
	{
		Error(eDLL_T::SERVER, "sv_kickid requires a UserID or OriginID. You can get the UserID with the 'status' command. Error: %s", e.what());
		return;
	}
}

/*
=====================
_Ban_f_CompletionFunc
=====================
*/
void _Ban_f_CompletionFunc(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		return;
	}

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		CBaseClient* pClient = g_pClient->GetClient(i);
		CNetChan* pNetChan = pClient->GetNetChan();

		if (!pClient || !pNetChan)
		{
			continue;
		}

		std::string svClientName = pNetChan->GetName(); // Get full name.

		if (svClientName.empty())
		{
			continue;
		}

		if (strcmp(args.Arg(1), svClientName.c_str()) != 0)
		{
			continue;
		}

		std::string svIpAddress = pNetChan->GetAddress(); // If this stays empty they modified the packet somehow.

		g_pBanSystem->AddEntry(svIpAddress, pClient->GetOriginID());
		g_pBanSystem->Save();
		NET_DisconnectClient(pClient, i, "Banned from Server", 0, 1);
	}
}

/*
=====================
_BanID_f_CompletionFunc
=====================
*/
void _BanID_f_CompletionFunc(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		return;
	}

	try
	{
		bool bOnlyDigits = args.HasOnlyDigits(1);
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			CBaseClient* pClient = g_pClient->GetClient(i);
			CNetChan* pNetChan = pClient->GetNetChan();

			if (!pClient || !pNetChan)
			{
				continue;
			}

			std::string svIpAddress = pNetChan->GetAddress(); // If this stays empty they modified the packet somehow.

			if (bOnlyDigits)
			{
				std::int64_t nTargetID = static_cast<std::int64_t>(std::stoll(args.Arg(1)));
				if (nTargetID > MAX_PLAYERS) // Is it a possible originID?
				{
					std::int64_t nOriginID = pClient->GetOriginID();
					if (nOriginID != nTargetID)
					{
						continue;
					}
				}
				else // If its not try by userID.
				{
					std::int64_t nClientID = static_cast<std::int64_t>(pClient->GetUserID() + 1); // Get UserID + 1.
					if (nClientID != nTargetID)
					{
						continue;
					}
				}

				g_pBanSystem->AddEntry(svIpAddress, pClient->GetOriginID());
				g_pBanSystem->Save();
				NET_DisconnectClient(pClient, i, "Banned from Server", 0, 1);
			}
			else
			{
				if (std::string(args.Arg(1)).compare(svIpAddress) != NULL)
				{
					continue;
				}

				g_pBanSystem->AddEntry(svIpAddress, pClient->GetOriginID());
				g_pBanSystem->Save();
				NET_DisconnectClient(pClient, i, "Banned from Server", 0, 1);
			}
		}
	}
	catch (std::exception& e)
	{
		Error(eDLL_T::SERVER, "Banid Error: %s", e.what());
		return;
	}
}

/*
=====================
_Unban_f_CompletionFunc
=====================
*/
void _Unban_f_CompletionFunc(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		return;
	}

	try
	{
		if (args.HasOnlyDigits(1)) // Check if we have an ip address or origin ID.
		{
			g_pBanSystem->DeleteEntry("noIP", std::stoll(args.Arg(1))); // Delete ban entry.
			g_pBanSystem->Save(); // Save modified vector to file.
		}
		else
		{
			g_pBanSystem->DeleteEntry(args.Arg(1), 1); // Delete ban entry.
			g_pBanSystem->Save(); // Save modified vector to file.
		}
	}
	catch (std::exception& e)
	{
		Error(eDLL_T::SERVER, "Unban Error: %s", e.what());
		return;
	}
}

/*
=====================
_ReloadBanList_f_CompletionFunc
=====================
*/
void _ReloadBanList_f_CompletionFunc(const CCommand& args)
{
	g_pBanSystem->Load(); // Reload banlist.
}

/*
=====================
 _Pak_ListPaks_f_CompletionFunc
=====================
*/
void _Pak_ListPaks_f_CompletionFunc(const CCommand& args)
{
#ifdef GAMEDLL_S3
	DevMsg(eDLL_T::RTECH, "| id | name                                               | status                               | asset count |\n");
	DevMsg(eDLL_T::RTECH, "|----|----------------------------------------------------|--------------------------------------|-------------|\n");

	std::uint32_t nActuallyLoaded = 0;

	for (int i = 0; i < *s_pLoadedPakCount; ++i)
	{
		RPakLoadedInfo_t info = g_pLoadedPakInfo[i];

		if (info.m_nStatus == RPakStatus_t::PAK_STATUS_FREED)
			continue;

		std::string rpakStatus = "RPAK_CREATED_A_NEW_STATUS_SOMEHOW";

		auto it = RPakStatusToString.find(info.m_nStatus);
		if (it != RPakStatusToString.end())
			rpakStatus = it->second;

		// todo: make status into a string from an array/vector
		DevMsg(eDLL_T::RTECH, "| %02i | %-50s | %-36s | %11i |\n", info.m_nPakId, info.m_pszFileName, rpakStatus.c_str(), info.m_nAssetCount);
		nActuallyLoaded++;
	}
	DevMsg(eDLL_T::RTECH, "|----|----------------------------------------------------|--------------------------------------|-------------|\n");
	DevMsg(eDLL_T::RTECH, "| %16i loaded paks.                                                                                |\n", nActuallyLoaded);
	DevMsg(eDLL_T::RTECH, "|----|----------------------------------------------------|--------------------------------------|-------------|\n");
#endif // GAMEDLL_S3
}

/*
=====================
 _Pak_RequestUnload_f_CompletionFunc
=====================
*/
void _Pak_RequestUnload_f_CompletionFunc(const CCommand& args)
{
#ifdef GAMEDLL_S3
	if (args.ArgC() < 2)
	{
		return;
	}

	try
	{
		if (args.HasOnlyDigits(1))
		{
			int nPakId = std::stoi(args.Arg(1));
			RPakLoadedInfo_t pakInfo = g_pRTech->GetPakLoadedInfo(nPakId);
			pakInfo.m_pszFileName ? DevMsg(eDLL_T::RTECH, "Requested Pak Unload for '%s'\n", pakInfo.m_pszFileName) : DevMsg(eDLL_T::RTECH, "Requested Pak Unload for '%d'\n", nPakId);
			RTech_UnloadPak(nPakId);
		}
		else
		{
			throw std::exception("Please provide a number as an arg.");
		}
	}
	catch (std::exception& e)
	{
		Error(eDLL_T::RTECH, "RequestUnload Error: %s", e.what());
		return;
	}
#endif // GAMEDLL_S3
}

/*
=====================
_Pak_RequestLoad_f_CompletionFunc
=====================
*/
void _Pak_RequestLoad_f_CompletionFunc(const CCommand& args)
{
	HRTech_AsyncLoad(args.Arg(1));
}

/*
=====================
_RTech_StringToGUID_f_CompletionFunc
=====================
*/
void _RTech_StringToGUID_f_CompletionFunc(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		return;
	}

	unsigned long long guid = g_pRTech->StringToGuid(args.Arg(1));

	DevMsg(eDLL_T::RTECH, "______________________________________________________________\n");
	DevMsg(eDLL_T::RTECH, "] RTECH_HASH -------------------------------------------------\n");
	DevMsg(eDLL_T::RTECH, "] GUID: '0x%llX'\n", guid);
}

/*
=====================
_RTech_Decompress_f_CompletionFunc

  Decompresses input RPak file and
  dumps results to 'paks\Win32\*.rpak'
=====================
*/
void _RTech_Decompress_f_CompletionFunc(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		return;
	}

	const std::string modDir = "paks\\Win32\\";
	const std::string baseDir = "paks\\Win64\\";

	std::string pakNameOut = modDir + args.Arg(1) + ".rpak";
	std::string pakNameIn = baseDir + args.Arg(1) + ".rpak";

	CreateDirectories(pakNameOut);

	DevMsg(eDLL_T::RTECH, "______________________________________________________________\n");
	DevMsg(eDLL_T::RTECH, "] RTECH_DECOMPRESS -------------------------------------------\n");

	if (!FileExists(pakNameIn.c_str()))
	{
		Error(eDLL_T::RTECH, "Error: pak file '%s' does not exist!\n", pakNameIn.c_str());
		return;
	}

	DevMsg(eDLL_T::RTECH, "] Processing: '%s'\n", pakNameIn.c_str());

	std::vector<std::uint8_t> upak; // Compressed region.
	std::ifstream ipak(pakNameIn, std::fstream::binary);

	ipak.seekg(0, std::fstream::end);
	upak.resize(ipak.tellg());
	ipak.seekg(0, std::fstream::beg);
	ipak.read((char*)upak.data(), upak.size());

	RPakHeader_t* rheader = (RPakHeader_t*)upak.data();
	uint16_t flags = (rheader->m_nFlags[0] << 8) | rheader->m_nFlags[1];

	DevMsg(eDLL_T::RTECH, "______________________________________________________________\n");
	DevMsg(eDLL_T::RTECH, "] HEADER_DETAILS ---------------------------------------------\n");
	DevMsg(eDLL_T::RTECH, "] Magic    : '%08X'\n", rheader->m_nMagic);
	DevMsg(eDLL_T::RTECH, "] Version  : '%u'\n", rheader->m_nVersion);
	DevMsg(eDLL_T::RTECH, "] Flags    : '%04X'\n", flags);
	DevMsg(eDLL_T::RTECH, "] Hash     : '%llu'\n", rheader->m_nHash);
	DevMsg(eDLL_T::RTECH, "] Entries  : '%zu'\n", rheader->m_nAssetEntryCount);
	DevMsg(eDLL_T::RTECH, "______________________________________________________________\n");
	DevMsg(eDLL_T::RTECH, "] COMPRESSION_DETAILS ----------------------------------------\n");
	DevMsg(eDLL_T::RTECH, "] Size disk: '%lld'\n", rheader->m_nSizeDisk);
	DevMsg(eDLL_T::RTECH, "] Size decp: '%lld'\n", rheader->m_nSizeMemory);
	DevMsg(eDLL_T::RTECH, "] Ratio    : '%.02f'\n", (rheader->m_nSizeDisk * 100.f) / rheader->m_nSizeMemory);

	if (rheader->m_nMagic != 'kaPR')
	{
		Error(eDLL_T::RTECH, "Error: pak file '%s' has invalid magic!\n", pakNameIn.c_str());
		return;
	}
	if ((rheader->m_nFlags[1] & 1) != 1)
	{
		Error(eDLL_T::RTECH, "Error: pak file '%s' already decompressed!\n", pakNameIn.c_str());
		return;
	}
	if (rheader->m_nSizeDisk != upak.size())
	{
		Error(eDLL_T::RTECH, "Error: pak file '%s' decompressed size '%zu' doesn't match expected value '%zu'!\n", pakNameIn.c_str(), upak.size(), rheader->m_nSizeMemory);
		return;
	}

	RPakDecompState_t state;
	std::uint32_t decompSize = g_pRTech->DecompressPakFileInit(&state, upak.data(), upak.size(), 0, PAK_HEADER_SIZE);

	if (decompSize == rheader->m_nSizeDisk)
	{
		Error(eDLL_T::RTECH, "Error: calculated size: '%zu' expected: '%zu'!\n", decompSize, rheader->m_nSizeMemory);
		return;
	}
	else
	{
		DevMsg(eDLL_T::RTECH, "] Calculated size: '%zu'\n", decompSize);
	}

	std::vector<std::uint8_t> pakBuf(rheader->m_nSizeMemory, 0);

	state.m_nOutMask = UINT64_MAX;
	state.m_nOut = uint64_t(pakBuf.data());

	std::uint8_t decompResult = g_pRTech->DecompressPakFile(&state, upak.size(), pakBuf.size());
	if (decompResult != 1)
	{
		Error(eDLL_T::RTECH, "Error: decompression failed for '%s' return value: '%u'!\n", pakNameIn.c_str(), +decompResult);
		return;
	}

	rheader->m_nFlags[1] = 0x0; // Set compressed flag to false for the decompressed pak file.
	rheader->m_nSizeDisk = rheader->m_nSizeMemory; // Equal compressed size with decompressed.

	std::ofstream outBlock(pakNameOut, std::fstream::binary);

	if (rheader->m_nPatchIndex > 0) // Check if its an patch rpak.
	{
		// Loop through all the structs and patch their compress size.
		for (int i = 1, patch_offset = 0x88; i <= rheader->m_nPatchIndex; i++, patch_offset += sizeof(RPakPatchCompressedHeader_t))
		{
			RPakPatchCompressedHeader_t* patch_header = (RPakPatchCompressedHeader_t*)((std::uintptr_t)pakBuf.data() + patch_offset);
			patch_header->m_nSizeDisk = patch_header->m_nSizeMemory; // Fix size for decompress.
		}
	}

	memcpy_s(pakBuf.data(), state.m_nDecompSize, ((std::uint8_t*)rheader), PAK_HEADER_SIZE); // Overwrite first 0x80 bytes which are NULL with the header data.

	outBlock.write((char*)pakBuf.data(), state.m_nDecompSize);

	uint32_t crc32_init = {};
	DevMsg(eDLL_T::RTECH, "] CRC32          : '%08X'\n", crc32::update(crc32_init, pakBuf.data(), state.m_nDecompSize));
	DevMsg(eDLL_T::RTECH, "] Decompressed rpak to: '%s'\n", pakNameOut.c_str());
	DevMsg(eDLL_T::RTECH, "--------------------------------------------------------------\n");

	outBlock.close();
}

/*
=====================
_NET_TraceNetChan_f_CompletionFunc

  Logs all data transmitted and received
  over the UDP socket to a file on the disk.
  File: '<mod\logs\net_trace.log>'.
=====================
*/
void _NET_TraceNetChan_f_CompletionFunc(const CCommand& args)
{
	static bool bTraceNetChannel = false;
	if (!bTraceNetChannel)
	{
		net_usesocketsforloopback->SetValue(1);
		DevMsg(eDLL_T::ENGINE, "\n");
		DevMsg(eDLL_T::ENGINE, "+--------------------------------------------------------+\n");
		DevMsg(eDLL_T::ENGINE, "|>>>>>>>>>>>>>| NETCHANNEL TRACE ACTIVATED |<<<<<<<<<<<<<|\n");
		DevMsg(eDLL_T::ENGINE, "+--------------------------------------------------------+\n");
		DevMsg(eDLL_T::ENGINE, "\n");

		// Begin the detour transaction to hook the the process.
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		NET_Trace_Attach();
		// Commit the transaction.
		if (DetourTransactionCommit() != NO_ERROR)
		{
			// Failed to hook into the process, terminate.
			TerminateProcess(GetCurrentProcess(), 0xBAD0C0DE);
		}
	}
	else
	{
		DevMsg(eDLL_T::ENGINE, "\n");
		DevMsg(eDLL_T::ENGINE, "+--------------------------------------------------------+\n");
		DevMsg(eDLL_T::ENGINE, "|>>>>>>>>>>>>| NETCHANNEL TRACE DEACTIVATED |<<<<<<<<<<<<|\n");
		DevMsg(eDLL_T::ENGINE, "+--------------------------------------------------------+\n");
		DevMsg(eDLL_T::ENGINE, "\n");

		// Begin the detour transaction to hook the the process.
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		NET_Trace_Detach();

		// Commit the transaction.
		DetourTransactionCommit();
	}
	bTraceNetChannel = !bTraceNetChannel;
}

/*
=====================
_VPK_Decompress_f_CompletionFunc

  Decompresses input VPK files and
  dumps the output to '<mod>\vpk'.
=====================
*/
void _VPK_Unpack_f_CompletionFunc(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		return;
	}
	std::string szPathOut = "platform\\vpk";
	std::chrono::milliseconds msStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

	DevMsg(eDLL_T::FS, "______________________________________________________________\n");
	DevMsg(eDLL_T::FS, "] FS_DECOMPRESS ----------------------------------------------\n");
	DevMsg(eDLL_T::FS, "] Processing: '%s'\n", args.Arg(1));

	VPKDir_t vpk = g_pPackedStore->GetPackDirFile(args.Arg(1));
	g_pPackedStore->InitLzDecompParams();

	std::thread th([&] { g_pPackedStore->UnpackAll(vpk, szPathOut); });
	th.join();

	std::chrono::milliseconds msEnd = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	float duration = msEnd.count() - msStart.count();

	DevMsg(eDLL_T::FS, "______________________________________________________________\n");
	DevMsg(eDLL_T::FS, "] OPERATION_DETAILS ------------------------------------------\n");
	DevMsg(eDLL_T::FS, "] Time elapsed: '%.3f' seconds\n", (duration / 1000));
	DevMsg(eDLL_T::FS, "] Decompressed vpk to: '%s'\n", szPathOut.c_str());
	DevMsg(eDLL_T::FS, "--------------------------------------------------------------\n");
}

/*
=====================
_VPK_Mount_f_CompletionFunc

  Mounts input VPK file for
  internal FileSystem usage
=====================
*/
void _VPK_Mount_f_CompletionFunc(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		return;
	}

	if (g_pFileSystem_Stdio)
	{
		VPKData_t* pPakData = g_pFileSystem_Stdio->MountVPK(args.Arg(1));
		if (pPakData)
		{
			DevMsg(eDLL_T::FS, "Mounted VPK file '%s' with handle '%d'\n", args.Arg(1), pPakData->m_nHandle);
		}
		else
		{
			Warning(eDLL_T::FS, "Unable to mount VPK file '%s': non-existent VPK file\n", args.Arg(1));
		}
	}
	else
	{
		Warning(eDLL_T::FS, "Unable to mount VPK file '%s': '%s' is not initalized\n", args.Arg(1), VAR_NAME(g_pFileSystem));
	}
}

/*
=====================
_NET_SetKey_f_CompletionFunc

  Sets the input netchannel encryption key
=====================
*/
void _NET_SetKey_f_CompletionFunc(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		return;
	}

	NET_SetKey(args.Arg(1));
}

/*
=====================
_NET_GenerateKey_f_CompletionFunc

  Sets a random netchannel encryption key
=====================
*/
void _NET_GenerateKey_f_CompletionFunc(const CCommand& args)
{
	NET_GenerateKey();
}
#ifndef DEDICATED
/*
=====================
_RCON_CmdQuery_f_CompletionFunc

  Issues an RCON command to the
  RCON server.
=====================
*/
void _RCON_CmdQuery_f_CompletionFunc(const CCommand& args)
{
	switch (args.ArgC())
	{
		case 0:
		case 1:
		{
			if (g_pRConClient->IsInitialized()
				&& !g_pRConClient->IsConnected()
				&& strlen(rcon_address->GetString()) > 0)
			{
				g_pRConClient->Connect();
			}
			break;
		}
		case 2:
		{
			if (!g_pRConClient->IsInitialized())
			{
				Warning(eDLL_T::CLIENT, "Failed to issue command to RCON server: uninitialized\n");
				break;
			}
			else if (g_pRConClient->IsConnected())
			{
				if (strcmp(args.Arg(1), "PASS") == 0) // Auth with RCON server using rcon_password ConVar value.
				{
					std::string svCmdQuery = g_pRConClient->Serialize(args.Arg(1), rcon_password->GetString(), cl_rcon::request_t::SERVERDATA_REQUEST_EXECCOMMAND);
					g_pRConClient->Send(svCmdQuery);
					break;
				}
				else if (strcmp(args.Arg(1), "disconnect") == 0) // Disconnect from RCON server.
				{
					g_pRConClient->Disconnect();
					break;
				}

				std::string svCmdQuery = g_pRConClient->Serialize(args.Arg(1), "", cl_rcon::request_t::SERVERDATA_REQUEST_EXECCOMMAND);
				g_pRConClient->Send(svCmdQuery);
				break;
			}
			else
			{
				Warning(eDLL_T::CLIENT, "Failed to issue command to RCON server: unconnected\n");
				break;
			}
			break;
		}
		case 3:
		{
			if (g_pRConClient->IsConnected())
			{
				if (strcmp(args.Arg(1), "PASS") == 0) // Auth with RCON server.
				{
					std::string svCmdQuery = g_pRConClient->Serialize(args.Arg(1), args.Arg(2), cl_rcon::request_t::SERVERDATA_REQUEST_AUTH);
					g_pRConClient->Send(svCmdQuery);
					break;
				}

				std::string svCmdQuery = g_pRConClient->Serialize(args.Arg(1), args.Arg(2), cl_rcon::request_t::SERVERDATA_REQUEST_SETVALUE);
				g_pRConClient->Send(svCmdQuery);
				break;
			}
			else
			{
				Warning(eDLL_T::CLIENT, "Failed to issue command to RCON server: unconnected\n");
				break;
			}
			break;
		}
	}
}

/*
=====================
_RCON_CmdQuery_f_CompletionFunc

  Disconnect from RCON server
=====================
*/
void _RCON_Disconnect_f_CompletionFunc(const CCommand& args)
{
	if (g_pRConClient->IsConnected())
	{
		g_pRConClient->Disconnect();
		DevMsg(eDLL_T::CLIENT, "User closed RCON connection\n");
	}
}
#endif // !DEDICATED

/*
=====================
_SQVM_ServerScript_f_CompletionFunc

  Exectutes input on the
  VM in SERVER context.
=====================
*/
void _SQVM_ServerScript_f_CompletionFunc(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		return;
	}

	SQVM_Execute(args.Arg(1), SQCONTEXT::SERVER);
}

#ifndef DEDICATED
/*
=====================
_SQVM_ClientScript_f_CompletionFunc

  Exectutes input on the
  VM in CLIENT context.
=====================
*/
void _SQVM_ClientScript_f_CompletionFunc(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		return;
	}

	SQVM_Execute(args.Arg(1), SQCONTEXT::CLIENT);
}

/*
=====================
_SQVM_UIScript_f_CompletionFunc

  Exectutes input on the
  VM in UI context.
=====================
*/
void _SQVM_UIScript_f_CompletionFunc(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		return;
	}

	SQVM_Execute(args.Arg(1), SQCONTEXT::UI);
}

/*
=====================
_IMaterial_GetMaterialAtCrossHair_f_CompletionFunc

  Print the material under the crosshair.
=====================
*/
void _CMaterial_GetMaterialAtCrossHair_f_ComplectionFunc(const CCommand& args)
{
#if defined (GAMEDLL_S3) // [ PIXIE ]: Verification needed for earlier seasons if CMaterialGlue matches.
	CMaterialGlue* material = GetMaterialAtCrossHair();
	if (material)
	{
		DevMsg(eDLL_T::MS, "______________________________________________________________\n");
		DevMsg(eDLL_T::MS, "] MATERIAL DETAILS -------------------------------------------\n");
		DevMsg(eDLL_T::MS, "] ADDR: '%llX'\n", material);
		DevMsg(eDLL_T::MS, "] GUID: '%llX'\n", material->m_GUID);
		DevMsg(eDLL_T::MS, "] UnknownSignature: '%d'\n", material->m_UnknownSignature);
		DevMsg(eDLL_T::MS, "] Material Width: '%d'\n", material->m_iWidth);
		DevMsg(eDLL_T::MS, "] Material Height: '%d'\n", material->m_iHeight);
		DevMsg(eDLL_T::MS, "] Flags: '%llX'\n", material->m_iFlags);

		std::function<void(CMaterialGlue*, const char*)> fnPrintChild = [](CMaterialGlue* material, const char* print)
		{
			DevMsg(eDLL_T::MS, "  ]___________________________________________________________\n");
			DevMsg(eDLL_T::MS, "  ] CHILD DETAILS --------------------------------------------\n");
			DevMsg(eDLL_T::MS, print, material);
			DevMsg(eDLL_T::MS, "    ] GUID: '%llX'\n", material->m_GUID);
			if (material->m_pszName)
			{
				DevMsg(eDLL_T::MS, "    ] Material Name: '%s'\n", material->m_pszName);
			}
			else
			{
				DevMsg(eDLL_T::MS, "    ] Material Name: 'NULL'\n");
			}

			DevMsg(eDLL_T::MS, "  ]___________________________________________________________\n");
		};

		if (material->m_pszName)
		{
			DevMsg(eDLL_T::MS, "] Material Name: '%s'\n", material->m_pszName);
		}
		else
		{
			DevMsg(eDLL_T::MS, "] Material Name: 'NULL'\n");
		}

		if (material->m_pszSurfaceName1)
		{
			DevMsg(eDLL_T::MS, "] Material Surface Name 1: '%s'\n", material->m_pszSurfaceName1);
		}
		else
		{
			DevMsg(eDLL_T::MS, "] Material Surface Name 1: 'NULL'\n");
		}

		if (material->m_pszSurfaceName2)
		{
			DevMsg(eDLL_T::MS, "] Material Surface Name 2: '%s'\n", material->m_pszSurfaceName2);
		}
		else
		{
			DevMsg(eDLL_T::MS, "] Material Surface Name 2: 'NULL'\n");
		}

		if (material->m_ppDXTexture1)
		{
			DevMsg(eDLL_T::MS, "] DX Texture 1: '%llX'\n", material->m_ppDXTexture1);
		}
		else
		{
			DevMsg(eDLL_T::MS, "] DX Texture 1: 'NULL'\n");
		}

		if (material->m_ppDXTexture2)
		{
			DevMsg(eDLL_T::MS, "] DX Texture 2: '%llX'\n", material->m_ppDXTexture2);
		}
		else
		{
			DevMsg(eDLL_T::MS, "] DX Texture 2: 'NULL'\n");
		}

		material->m_pDepthShadow  ? fnPrintChild(material->m_pDepthShadow,  "  ] DepthShadow Addr: '%llX'\n")      : DevMsg(eDLL_T::MS, "] DepthShadow Addr: 'NULL'\n");
		material->m_pDepthPrepass ? fnPrintChild(material->m_pDepthPrepass, "  ] DepthPrepass Addr: '%llX'\n")     : DevMsg(eDLL_T::MS, "] DepthPrepass Addr: 'NULL'\n");
		material->m_pDepthVSM     ? fnPrintChild(material->m_pDepthVSM,     "  ] DepthVSM Addr: '%llX'\n")         : DevMsg(eDLL_T::MS, "] DepthVSM Addr: 'NULL'\n");
		material->m_pDepthShadow  ? fnPrintChild(material->m_pDepthShadow,  "  ] DepthShadowTight Addr: '%llX'\n") : DevMsg(eDLL_T::MS, "] DepthShadowTight Addr: 'NULL'\n");
		material->m_pColPass      ? fnPrintChild(material->m_pColPass,      "  ] ColPass Addr: '%llX'\n")          : DevMsg(eDLL_T::MS, "] ColPass Addr: 'NULL'\n");

		DevMsg(eDLL_T::MS, "  ]___________________________________________________________\n");
		DevMsg(eDLL_T::MS, "  ] TEXTURE GUID MAP DETAILS ---------------------------------\n");
		material->m_pTextureGUID1 ? DevMsg(eDLL_T::MS, "    ] TextureMap 1 Addr: '%llX'\n", material->m_pTextureGUID1) : DevMsg(eDLL_T::MS, "   ] TextureMap 1 Addr: 'NULL'\n");
		material->m_pTextureGUID2 ? DevMsg(eDLL_T::MS, "    ] TextureMap 2 Addr: '%llX'\n", material->m_pTextureGUID2) : DevMsg(eDLL_T::MS, "   ] TextureMap 2 Addr: 'NULL'\n");
		DevMsg(eDLL_T::MS, "  ]___________________________________________________________\n");

		DevMsg(eDLL_T::MS, "--------------------------------------------------------------\n");
	}
	else
	{
		DevMsg(eDLL_T::MS, "No Material found >:(\n");
	}
#endif
}
#endif // !DEDICATED