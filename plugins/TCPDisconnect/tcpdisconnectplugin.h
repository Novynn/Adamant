#ifndef TCPDISCONNECTPLUGIN_H
#define TCPDISCONNECTPLUGIN_H

#include "adamantplugin.h"
#include <winsock2.h>
#include <windows.h>
#include <Ws2tcpip.h>
#include <iphlpapi.h>
#include <iprtrmib.h>
#include <stdio.h>
#include <psapi.h>
#include <QHostAddress>
class PluginManager;

class TcpDisconnectPlugin : public AdamantPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.adamant.plugin.tcpdisconnect" FILE "tcpdisconnect.json")
    Q_INTERFACES(AdamantPlugin)

public:
    static bool disconnect() {
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
                QHostAddress addr(ntohl(owner->dwRemoteAddr));
                bool isPathOfExile = false;

                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, owner->dwOwningPid);
                {
                    if (hProcess != NULL) {
                        HMODULE hMod;
                        DWORD cbNeeded;
                        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
                            wchar_t szProcessName[MAX_PATH];
                            GetModuleBaseName(hProcess, hMod, szProcessName, MAX_PATH);
                            isPathOfExile = QString("PathOfExileSteam.exe").compare(QString::fromWCharArray(szProcessName)) == 0;
                        }
                    }
                }
                CloseHandle(hProcess);

                if (isPathOfExile) {
                    owner->dwState = MIB_TCP_STATE_DELETE_TCB;
                    result = SetTcpEntry((MIB_TCPROW*)owner);
                }
            }
        }

        return (result == 0);
    }

    static QScriptValue disconnect(QScriptContext *context , QScriptEngine *engine) {
        Q_UNUSED(context);
        bool success = TcpDisconnectPlugin::disconnect();
        return engine->toScriptValue(success);
    }
public slots:
    void SetupEngine(QScriptEngine* engine, QScriptValue* plugin) {
        plugin->setProperty("disconnect", engine->newFunction(TcpDisconnectPlugin::disconnect));
    }
};

#endif // TCPDISCONNECTPLUGIN_H

