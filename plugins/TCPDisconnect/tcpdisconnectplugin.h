#ifndef TCPDISCONNECTPLUGIN_H
#define TCPDISCONNECTPLUGIN_H

#include "adamantplugin.h"
#ifdef Q_OS_WIN32
#include <winsock2.h>
#include <windows.h>
#include <Ws2tcpip.h>
#include <iphlpapi.h>
#include <iprtrmib.h>
#include <psapi.h>
#endif
class PluginManager;

class TcpDisconnectPlugin : public AdamantPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "adamant.tcpdisconnect" FILE "tcpdisconnect.json")
    Q_INTERFACES(AdamantPlugin)

public:
    static QString getProcessNameByPID(DWORD pid) {
        QString result;
#ifdef Q_OS_WIN32
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        {
            if (hProcess != NULL) {
                HMODULE hMod;
                DWORD cbNeeded;
                if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
                    wchar_t szProcessName[MAX_PATH];
                    GetModuleBaseName(hProcess, hMod, szProcessName, MAX_PATH);
                    result = QString::fromWCharArray(szProcessName);
                }
            }
        }
        CloseHandle(hProcess);
#endif
        return result;
    }

    static bool disconnect() {
#ifdef Q_OS_WIN32
        static QStringList processes = {"PathOfExile.exe", "PathOfExileSteam.exe"};
        MIB_TCPTABLE_OWNER_PID* table;
        DWORD size;
        DWORD result;
        result = GetExtendedTcpTable(NULL, &size, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL , 0);
        table = (MIB_TCPTABLE_OWNER_PID*) malloc(size);
        result = GetExtendedTcpTable(table, &size, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL , 0);
        if (result == NO_ERROR) {
            result = -1;
            for (DWORD dwLoop = 0; dwLoop < table->dwNumEntries; dwLoop++) {
                MIB_TCPROW_OWNER_PID *owner = &table->table[dwLoop];
                if (owner->dwState != MIB_TCP_STATE_ESTAB) continue;
                bool isPathOfExile = (processes.contains(TcpDisconnectPlugin::getProcessNameByPID(owner->dwOwningPid), Qt::CaseInsensitive));
                if (isPathOfExile) {
                    owner->dwState = MIB_TCP_STATE_DELETE_TCB;
                    result = SetTcpEntry((MIB_TCPROW*)owner);
                }
            }
        }
        return (result == 0);
#else
        return false;
#endif
    }

    static QScriptValue disconnect(QScriptContext *context , QScriptEngine *engine) {
        Q_UNUSED(context);
#ifdef Q_OS_WIN32
        bool success = TcpDisconnectPlugin::disconnect();
        return engine->toScriptValue(success);
#else
        return context->throwError(QScriptContext::UnknownError, "The TCP Disconnect plugin currently only works on Windows.");
#endif
    }
public slots:
    void setupEngine(QScriptEngine* engine, QScriptValue* plugin) {
        plugin->setProperty("disconnect", engine->newFunction(TcpDisconnectPlugin::disconnect));
    }
};

#endif // TCPDISCONNECTPLUGIN_H

