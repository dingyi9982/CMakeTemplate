#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "RRLogger.h"
#include "demo.h"

#ifdef __x86_64__
#include "base/files/file_path.h"
#include "client/crash_report_database.h"
#include "client/crashpad_client.h"
#include "client/settings.h"
#if defined(__APPLE__)
using StringType = std::string;
#elif defined(__linux__)
using StringType = std::string;
#elif defined(_MSC_VER)
using StringType = std::wstring;
#endif

StringType getExecutableDir() {
  char pBuf[FILENAME_MAX];
  int len = sizeof(pBuf);
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
  int bytes = MIN(readlink("/proc/self/exe", pBuf, len), len - 1);
  if (bytes >= 0) {
    pBuf[bytes] = '\0';
  }

  char* lastForwardSlash = strrchr(&pBuf[0], '/');
  if (lastForwardSlash == nullptr) {
    return "";
  }
  *lastForwardSlash = '\0';

  return pBuf;
}

bool initializeCrashpad() {
  // Get directory where the exe lives so we can pass a full path to handler, reportsDir, metricsDir
  // and attachments
  StringType exeDir = getExecutableDir();

  // Ensure that handler is shipped with your application
  base::FilePath handler("../../Common/third_party/crashpad/crashpad_handler");

  // Directory where reports will be saved. Important! Must be writable or crashpad_handler will
  // crash.
  base::FilePath reportsDir(exeDir + "/reports");

  // Directory where metrics will be saved. Important! Must be writable or crashpad_handler will
  // crash.
  base::FilePath metricsDir(exeDir + "/metrics");

  // Configure url with BugSplat’s public fred database. Replace 'fred' with the name of your
  // BugSplat database. StringType url = "https://fred.bugsplat.com/post/bp/crash/crashpad.php";
  StringType url;

  // Metadata that will be posted to the server with the crash report map
  std::map<StringType, StringType> annotations;
  annotations["format"] = "minidump";  // Required: Crashpad setting to save crash as a minidump
  annotations["database"] = "fred";    // Required: BugSplat appName
  annotations["product"] = "myCrashpadCrasher";        // Required: BugSplat appName
  annotations["version"] = "1.0.0";                    // Required: BugSplat appVersion
  annotations["key"] = "Sample key";                   // Optional: BugSplat key field
  annotations["user"] = "fred@bugsplat.com";           // Optional: BugSplat user email
  annotations["list_annotations"] = "Sample comment";  // Optional: BugSplat crash description

  // Disable crashpad rate limiting so that all crashes have dmp files
  std::vector<StringType> arguments;
  arguments.emplace_back("--no-rate-limit");

  // Initialize Crashpad database
  std::unique_ptr<crashpad::CrashReportDatabase> database
      = crashpad::CrashReportDatabase::Initialize(reportsDir);
  if (database == nullptr) {
    return false;
  }

  // File paths of attachments to uploaded with minidump file at crash time - default upload limit
  // is 2MB
  std::vector<base::FilePath> attachments;
  // base::FilePath attachment(exeDir + "/attachment.txt");
  // attachments.emplace_back(attachment);

  // Enable automated crash uploads
  crashpad::Settings* settings = database->GetSettings();
  if (settings == nullptr) {
    return false;
  }
  // settings->SetUploadsEnabled(true);
  settings->SetUploadsEnabled(false);

  // Start crash handler
  crashpad::CrashpadClient* client = new crashpad::CrashpadClient();
  bool status = client->StartHandler(handler, reportsDir, metricsDir, url, annotations, arguments,
                                     true, true, attachments);
  delete client;    // todo: 是否可以delete???
  return status;
}
#endif

int main() {
#ifdef __x86_64__
  if (!initializeCrashpad()) {
    std::cout << "initialize Crashpad failed!" << std::endl;
  }
#endif

  RRLogger::InitLog(true, "debug", "./log");

  RRLOG_DEBUG("{}: debug log", 1);
  RRLOG_INFO("{}: info log", 2);
  RRLOG_WARN("{}: warning log", 3);
  RRLOG_ERROR("{}: error log", 4);

  RRLOG_INFO("GetData: {}", GetData());

  return 0;
}
