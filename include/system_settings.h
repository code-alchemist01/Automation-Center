#ifndef SYSTEM_SETTINGS_H
#define SYSTEM_SETTINGS_H

#include <stdio.h>
#include <windows.h>


// Main function to run the system settings module
void run_system_settings();

// Menu and display functions
void show_system_settings_menu();

// Core functionality functions
void view_system_info();
void manage_startup_programs();
void configure_power_settings();
void manage_environment_variables();
void configure_system_performance();
void backup_restore_registry();
void view_installed_programs();
void configure_user_accounts();
void system_maintenance();

// Registry helper functions
int read_registry_string(HKEY hKey, const char* subKey, const char* valueName, char* buffer, DWORD bufferSize);
int write_registry_string(HKEY hKey, const char* subKey, const char* valueName, const char* value);
int delete_registry_value(HKEY hKey, const char* subKey, const char* valueName);

#endif // SYSTEM_SETTINGS_H