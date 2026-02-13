#ifndef CLI_H
#define CLI_H

typedef enum {
  Z_GET_OPT_BOOL,
  Z_GET_OPT_INT,
  Z_GET_OPT_DOUBLE,
  Z_GET_OPT_STRING,
} Z_CLI_Option_Type;

typedef enum {
  Z_GET_OPT_REQUIRED,
  Z_GET_OPT_OPTIONAL,
} Z_CLI_Option_Status;

typedef struct {
  Z_Set flags;
  const char *description;
  void *value;
  Z_CLI_Option_Type type;
  Z_CLI_Option_Status status;
} Z_CLI_Option;

typedef struct {
  Z_Heap *heap;
  Z_CLI_Option *ptr;
  size_t length;
  size_t capacity;
} Z_CLI_Option_Array;

// // void get_opt("-h|--help", "this is help information", &is_help, Z_GET_OPT_BOOL);
// void get_opt(int argc, char **argv, const char *flags, const char *description, void *value, Z_CLI_Option_Type type, Z_CLI_Option_Status status, ...)
// {
//   Z_Heap_Auto heap = {0};;
//   Z_CLI_Option_Array options = z_array_new(&heap, Z_CLI_Option_Array);
//   // TODO: parsing to array
//   get_opt_array(&options, argc, argv);
// }

// void get_opt_array(int argc, char **argv, const Z_CLI_Option_Array *options)
// {

// }

#endif