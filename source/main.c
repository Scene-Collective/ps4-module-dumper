#include "ps4.h"

char output_root[255] = { 0 };

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

  char fw_version[6] = { 0 };
  char usb_name[64] = { 0 };
  char usb_path[64] = { 0 };
  char completion_check[255] = { 0 };

  initKernel();
  initLibc();

  jailbreak();
  mmap_patch();

  initSysUtil();

  get_firmware_string(fw_version);

  printf_notification("PS4 Module Dumper");

  if (!wait_for_usb(usb_name, usb_path)) {
    printf_notification("Waiting for USB drive...");
    do {
      sceKernelSleep(2);
    } while (!wait_for_usb(usb_name, usb_path));
  }

  sprintf(output_root, "%s/PS4", usb_path, fw_version);
  mkdir(output_root, 0777);
  sprintf(output_root, "%s/%s", output_root, fw_version);

  sprintf(completion_check, "%s/.complete", output_root);
  if (file_exists(completion_check)) {
    printf_notification("Modules already dumped for %s, skipping dumping", fw_version);
    return 0;
  } else {
    rmtree(output_root);
  }

  mkdir(output_root, 0777);

  printf_notification("Dumping Modules\nFW Version: %s\n Location: %s", fw_version, output_root);
  sceKernelSleep(5);

  /*
  dump_dir_macro("/system", "%s/system");
  dump_dir_macro("/system_ex", "%s/system_ex");
  dump_dir_macro("/update", "%s/update");
  dump_file_macro("/mini-syscore.elf", "%s/mini-syscore.elf");
  dump_file_macro("/safemode.elf", "%s/safemode.elf");
  dump_file_macro("/SceSysAvControl.elf", "%s/SceSysAvControl.elf");
  */

  // cleanup(output_root); // TODO: Recursively delete EMPTY directories from output_root

  touch_file(completion_check);

  printf_notification("Done!");

  return 0;
}
