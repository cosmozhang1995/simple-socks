#include "{{header_file_path}}"

## if module_names

#include <memory.h>
#include <stdlib.h>

## endif


## for module_name in module_names

{{module_name}}_t *{{module_name}}_create()
{
    {{module_name}}_t *instance = malloc(sizeof({{module_name}}_t));
    {{module_name}}_initialize(instance);
    return instance;
}

void {{module_name}}_destroy({{module_name}}_t *instance)
{
    {{module_name}}_uninitialize(instance);
    free(instance);
}

void {{module_name}}_initialize({{module_name}}_t *instance)
{
    memset(instance, 0, sizeof({{module_name}}_t));
}

void {{module_name}}_uninitialize({{module_name}}_t *instance)
{
}

## endfor
