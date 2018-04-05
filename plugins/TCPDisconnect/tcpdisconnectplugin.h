#ifndef TCPDISCONNECTPLUGIN_H
#define TCPDISCONNECTPLUGIN_H

#include "adamantplugin.h"
#ifdef Q_OS_WIN32
#define WIN32_LEAN_AND_MEAN
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
    static bool isElevated() {
        bool elevated = false;
#ifdef Q_OS_WIN32
        HANDLE hToken = NULL;
        if(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
            TOKEN_ELEVATION Elevation;
            DWORD cbSize = sizeof(TOKEN_ELEVATION);
            if(GetTokenInformation( hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
                elevated = Elevation.TokenIsElevated;
            }
        }
        if(hToken) {
            CloseHandle(hToken);
        }
#endif
        return elevated;
    }

    static QString getProcessNameByPID(DWORD pid) {
        QString result;
#ifdef Q_OS_WIN32
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess != NULL) {
            wchar_t szProcessName[MAX_PATH];
            GetModuleFileNameEx(hProcess, NULL, szProcessName, MAX_PATH);
            result = QString::fromWCharArray(szProcessName).section('\\', -1, -1);
            CloseHandle(hProcess);
        }
#endif
        return result;
    }

    static bool disconnect() {
#ifdef Q_OS_WIN32
        static QStringList processes = {
            "PathOfExile.exe",
            "PathOfExileSteam.exe",
            "PathOfExile_x64.exe",
            "PathOfExile_x64Steam.exe",
        };
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
                const QString &name = TcpDisconnectPlugin::getProcessNameByPID(owner->dwOwningPid);

                bool isPathOfExile = (!name.isEmpty() && processes.contains(name, Qt::CaseInsensitive));
                if (isPathOfExile) {
                    owner->dwState = MIB_TCP_STATE_DELETE_TCB;
                    result = SetTcpEntry((MIB_TCPROW*)owner);
                }
            }
        }

        free(table);

        return (result == 0);
#else
        return false;
#endif
    }

    static QScriptValue disconnect(QScriptContext *context, QScriptEngine *engine, void* arg) {
        auto plugin = reinterpret_cast<TcpDisconnectPlugin*>(arg);
        Q_UNUSED(context);
        Q_UNUSED(plugin);
#ifdef Q_OS_WIN32
        bool success = TcpDisconnectPlugin::disconnect();
        return engine->toScriptValue(success);
#else
        return context->throwError(QScriptContext::UnknownError, "The TCP Disconnect plugin currently only works on Windows.");
#endif
    }

    static QScriptValue isElevated(QScriptContext *context, QScriptEngine *engine) {
        Q_UNUSED(context);
        return engine->toScriptValue(isElevated());
    }

public slots:
    void setupEngine(QScriptEngine* engine, QScriptValue* plugin) {
        plugin->setProperty("disconnect", engine->newFunction(TcpDisconnectPlugin::disconnect, this));
        plugin->setProperty("isElevated", engine->newFunction(TcpDisconnectPlugin::isElevated));
    }
};

#endif // TCPDISCONNECTPLUGIN_H

