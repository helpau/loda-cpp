#pragma once

#include <string>

class Http {
 public:
  static bool get(const std::string &url, const std::string &local_path,
                  bool fail_on_error = true);

  static bool postFile(const std::string &url, const std::string &file_path,
                       const std::string &auth = std::string());

 private:
  static int64_t WWW_CLIENT;
  static void initWWWClient();
};

bool isFile(const std::string &path);

bool isDir(const std::string &path);

void ensureDir(const std::string &path);

int64_t getFileAgeInDays(const std::string &path);

size_t getMemUsage();

class FolderLock {
 public:
  FolderLock(std::string folder);

  ~FolderLock();

  void release();

 private:
  std::string lockfile;
  int fd;
};
