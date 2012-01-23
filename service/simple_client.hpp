#pragma once
#include <nscapi/nscapi_helper.hpp>

class NSClientT;
namespace nsclient {
	class simple_client {
		NSClient *core_;
	public:
		simple_client(NSClient *core) : core_(core) {}

		void log(std::wstring msg) {
			std::string s = nsclient::logger_helper::create_info(__FILE__, __LINE__, msg); 
			core_->reportMessage(s);
		}
		void start() {
			if (!core_->boot_init()) {
				core_->log_error(__FILE__, __LINE__, _T("Service failed to init"));
				return;
			}
			if (!core_->boot_load_all_plugins()) {
				core_->log_error(__FILE__, __LINE__, _T("Service failed to load plugins"));
				return;
			}
			if (!core_->boot_start_plugins(true)) {
				core_->log_error(__FILE__, __LINE__, _T("Service failed to start plugins"));
				return;
			}

			if (core_->get_service_control().is_started())
				core_->log_info(__FILE__, __LINE__, _T("Service seems to be started (Sockets and such will probably not work)..."));

			//std::wcout << _T("Using settings from: ") << settings_manager::get_core()->get_settings_type_desc() << std::endl;
			core_->log_info(__FILE__, __LINE__, _T("Enter command to inject or exit to terminate..."));
/*
			Settings::get_settings()->clear_cache();
			LOG_MESSAGE_STD( _T("test 001: ") + SETTINGS_GET_STRING(NSCLIENT_TEST1) );
			LOG_MESSAGE_STD( _T("test 002: ") + SETTINGS_GET_STRING(NSCLIENT_TEST2) );
			LOG_MESSAGE_STD( _T("test 003: ") + SETTINGS_GET_STRING(NSCLIENT_TEST3) );
			LOG_MESSAGE_STD( _T("test 004: ") + SETTINGS_GET_STRING(NSCLIENT_TEST4) );

			Settings::get_settings()->save_to(_T("test.ini"));
*/
			std::wstring s = _T("");

			while (true) {
				std::wstring s;
				std::getline(std::wcin, s);
				if (s == _T("exit")) {
					log(_T("Exiting..."));
					break;
				} else if (s == _T("plugins")) {
					log(_T("Plugins: "));
					core_->listPlugins();
				} else if (s == _T("list") || s == _T("commands")) {
					log(_T("Commands:"));
					std::list<std::wstring> lst = core_->list_commands();
					for (std::list<std::wstring>::const_iterator cit = lst.begin(); cit!=lst.end();++cit)
						log(_T("| ") + *cit + _T(": ") + core_->describeCommand(*cit));
				} else if (s.size() > 4 && s.substr(0,3) == _T("log")) {
					log(_T("Setting log to: ") + s.substr(4));
					core_->set_loglevel(s.substr(4));
				} else if (s == _T("reattach")) {
					log(_T("Reattaching to session 0"));
					core_->startTrayIcon(0);
				} else if (s == _T("assert")) {
					int *foo = 0;
					*foo = 0;
					throw "test";
				} else {
					try {
						strEx::token t = strEx::getToken(s, ' ');
						std::wstring msg, perf;
						NSCAPI::nagiosReturn ret = core_->inject(t.first, t.second, msg, perf);
						if (ret == NSCAPI::returnIgnored) {
							log(_T("No handler for command: ") + t.first);
						} else {
							log(nscapi::plugin_helper::translateReturn(ret) + _T(":") + msg);
							if (!perf.empty())
								log(_T(" Performance data: ") + perf);
						}
					} catch (const nscapi::nscapi_exception &e) {
						log(_T("NSCAPI Exception: ") + utf8::cvt<std::wstring>(e.what()));
					} catch (const std::exception &e) {
						log(_T("Exception: ") + utf8::cvt<std::wstring>(e.what()));
					} catch (...) {
						log(_T("Unknown exception"));
					}
				}
			}
			core_->stop_unload_plugins_pre();
			core_->stop_exit_pre();
			core_->stop_unload_plugins_post();
			core_->stop_exit_post();
		}
	};
}