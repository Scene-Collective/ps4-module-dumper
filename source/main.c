#include "ps4.h"

char output_root[255] = {0};
int nthread_run;
char notify_buf[512];

void *nthread_func(void *arg) {
  UNUSED(arg);
  time_t t1, t2;
  t1 = 0;
  while (nthread_run) {
    if (notify_buf[0]) {
      t2 = time(NULL);
      if ((t2 - t1) >= 20) {
        t1 = t2;
        systemMessage(notify_buf);
      }
    } else {
      t1 = 0;
    }
    sceKernelSleep(1);
  }

  return NULL;
}

int dump_dir_macro(char *src, char *dest) {
  char output_loc[255];
  printf_notification("Dumping %s", src);
  sprintf(output_loc, dest, output_root);
  decrypt_dir(src, output_loc);

  return 0;
}

int dump_file_macro(char *src, char *dest) {
  char output_loc[255];
  printf_notification("Dumping %s", src);
  sprintf(output_loc, dest, output_root);
  decrypt_and_dump_self(src, output_loc);

  return 0;
}

int _main(struct thread *td) {
  UNUSED(td);

  char fw_version[6] = {0};
  char usb_name[64] = {0};
  char usb_path[64] = {0};
  char directory_base[255] = {0};
  char firmware_base[255] = {0};
  char completion_check[255] = {0};

  initKernel();
  initLibc();
  initPthread();

  jailbreak();
  mmap_patch();

  initSysUtil();

  get_firmware_string(fw_version);

  nthread_run = 1;
  notify_buf[0] = '\0';
  ScePthread nthread;
  scePthreadCreate(&nthread, NULL, nthread_func, NULL, "nthread");

  printf_notification("Running Module Dumper");

  if (!wait_for_usb(usb_name, usb_path)) {
    sprintf(notify_buf, "Waiting for USB device...");
    do {
      sceKernelSleep(1);
    } while (!wait_for_usb(usb_name, usb_path));
    notify_buf[0] = '\0';
  }

  sprintf(directory_base, "%s/PS4", usb_path);
  mkdir(directory_base, 0777);
  sprintf(firmware_base, "%s/%s", directory_base, fw_version);
  mkdir(firmware_base, 0777);
  sprintf(output_root, "%s/modules", firmware_base);

  sprintf(completion_check, "%s/.complete", output_root);
  if (file_exists(completion_check)) {
    printf_notification("Modules already dumped for %s, skipping dumping", fw_version);
    return 0;
  } else {
    rmtree(output_root);
  }

  mkdir(output_root, 0777);

  printf_notification("USB device detected.\n\nStarting module dumping to %s.", usb_name);

  dump_dir_macro("/system", "%s/system");
  dump_dir_macro("/system_ex", "%s/system_ex");
  dump_dir_macro("/update", "%s/update");
  dump_file_macro("/mini-syscore.elf", "%s/mini-syscore.elf");
  dump_file_macro("/safemode.elf", "%s/safemode.elf");
  dump_file_macro("/SceSysAvControl.elf", "%s/SceSysAvControl.elf");

  //cleanup(output_root);

  touch_file(completion_check);

  printf_notification("Modules dumped successfully!");

  nthread_run = 0;

  return 0;
}
