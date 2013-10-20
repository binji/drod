#include <assert.h>
#include <fcntl.h>
#include <libtar.h>
#include <SDL_main.h>
#include <stdio.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>

extern int drod_main(int argc, char *argv[]);

static void extract_tar(char* src, char* dst) {
  TAR* tar;
  int ret = tar_open(&tar, src, NULL, O_RDONLY, 0, 0);
  assert(ret == 0);

  ret = tar_extract_all(tar, dst);
  assert(ret == 0);

  ret = tar_close(tar);
  assert(ret == 0);
}

int main(int argc, char* argv[]) {
  umount("/");
  mount("", "/", "memfs", 0, NULL);
  mount("", "/var", "html5fs", 0, "type=PERSISTENT");
  mount("", "/tars", "httpfs", 0, NULL);

  extract_tar("/tars/drod_usr.tar", "/");  // read-only data

  struct stat statbuf;
  // Test if an expected file is there...
  int ret = stat("/var/games/drod/drod1_6.dat", &statbuf);  // read/write data.
  if (ret != 0) {
    extract_tar("/tars/drod_var.tar", "/");
  }

  setenv("DROD_1_6_DAT_PATH", "/var/games/drod", 1);
  setenv("DROD_1_6_RES_PATH", "/usr/local/share/games/drod", 1);

  return drod_main(argc, argv);
}
