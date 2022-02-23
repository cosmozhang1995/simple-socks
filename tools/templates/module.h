#ifndef {{module_def}}
#define {{module_def}}

## for module_name in module_names

typedef struct {{module_name}}_s {{module_name}}_t;

struct {{module_name}}_s {
};

{{module_name}}_t *{{module_name}}_create();
void {{module_name}}_destroy({{module_name}}_t *);
void {{module_name}}_initialize({{module_name}}_t *);
void {{module_name}}_uninitialize({{module_name}}_t *);

## endfor

#endif // {{module_def}}
