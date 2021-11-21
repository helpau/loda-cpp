#pragma once

#include <string>

bool isFile(const std::string &path);

bool isDir(const std::string &path);

void ensureDir(const std::string &path);

void moveDir(const std::string &from, const std::string &to);

void gunzip(const std::string &path);

void ensureTrailingSlash(std::string &dir);

std::string getHomeDir();

std::string getTmpDir();

std::string getFileAsString(const std::string &filename);

int64_t getFileAgeInDays(const std::string &path);

size_t getMemUsage();

bool hasGit();

class FolderLock {
 public:
  FolderLock(std::string folder);

  ~FolderLock();

  void release();

 private:
  std::string lockfile;
  int fd;
};
