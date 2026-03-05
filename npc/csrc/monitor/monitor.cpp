#include "common.h"
#include "memory/pmem.h"
#include "utils.h"

void init_log(const char *log_file);
static char *log_file = NULL;

static char *img_file = NULL;
int parse_args(int argc, char *argv[]) {
  const struct option table[] = {
      {"batch", no_argument, NULL, 'b'},
      {"log", required_argument, NULL, 'l'},
      {"diff", required_argument, NULL, 'd'},
      {"port", required_argument, NULL, 'p'},
      {"help", no_argument, NULL, 'h'},
      {"ftrace", required_argument, NULL, 'f'},
      {0, 0, NULL, 0},
  };
  int o;
  while ((o = getopt_long(argc, argv, "-bhl:d:p:f:", table, NULL)) != -1) {
    switch (o) {
    case 'b':
      // sdb_set_batch_mode();
      break;
    case 'p':
      // sscanf(optarg, "%d", &difftest_port);
      break;
    case 'l':
      log_file = optarg;
      INFO("LOG: %s", log_file);
      break;
    case 'd':
      // diff_so_file = optarg;
      break;
    case 'f':
      // elf_file = optarg;
      break;
    case 1:
      img_file = optarg;
      INFO("IMG: %s", img_file);
      return 0;
    default:
      printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
      printf("\t-b,--batch              run with batch mode\n");
      printf("\t-l,--log=FILE           output log to FILE\n");
      printf("\t-f,--ftrace=ELF_FILE    input elf file    \n");
      printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
      printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
      printf("\n");
      exit(0);
    }
  }
  return 0;
}

long load_img() {
  if (img_file == NULL) {
    Log("No image is given. Use the default build-in image.");
    return 4096; // built-in image size
  }

  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "Can not open '%s'", img_file);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);

  Log("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  Assert(ret == 1, "image read error");

  fclose(fp);
  return size;
}

FILE *log_fp = NULL;
void init_log(/*const char *log_file*/) {
  log_fp = stdout;
  if (log_file != NULL) {
    FILE *fp = fopen(log_file, "w");
    Assert(fp, "Can not open '%s'", log_file);
    log_fp = fp;
  }
  Log("Log is written to %s", log_file ? log_file : "stdout");
}

bool log_enable() {
  // return MUXDEF(CONFIG_TRACE,
  //               (g_nr_guest_inst >= CONFIG_TRACE_START) &&
  //                   (g_nr_guest_inst <= CONFIG_TRACE_END),
  //               false);

  // TODO: detact the instruction step to make it change
  return true;
}
