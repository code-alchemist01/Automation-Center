#ifndef REPORTS_H
#define REPORTS_H

#include <stdio.h>

// Main function to run the reports module
void run_reports();

// Menu display function
void show_reports_menu();

// Report generation functions
void generate_system_report();
void generate_security_html_report();
void generate_performance_report();
void generate_backup_report();
void generate_network_report();
void generate_comprehensive_report();

// Report management functions
void view_existing_reports();
void export_report_to_pdf();

// Helper functions for report generation
void create_reports_directory();
void write_html_header(FILE* file, const char* title);
void write_html_footer(FILE* file);

// Data collection functions for reports
void get_system_info_for_report(char* buffer, int bufferSize);
void get_disk_info_for_report(char* buffer, int bufferSize);
void get_memory_info_for_report(char* buffer, int bufferSize);
void get_network_info_for_report(char* buffer, int bufferSize);
void get_process_info_for_report(char* buffer, int bufferSize);
void get_log_analysis_for_report(char* buffer, int bufferSize);
void get_backup_status_for_report(char* buffer, int bufferSize);

#endif // REPORTS_H