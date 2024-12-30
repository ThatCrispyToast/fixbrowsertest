/*
 * FixBrowser v0.1 - https://www.fixbrowser.org/
 * Copyright (c) 2018-2024 Martin Dvorak <jezek2@advel.cz>
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, 
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <winsock.h>
#else
#include <signal.h>
#endif
#include "fixio.h"
#include "fiximage.h"
#include "fixgui.h"
#include "fixtask.h"
#include "browser.h"
#include "util.h"
#include "embed_scripts.h"
#include "embed_resources_proxy.h"

enum {
   SM_selector,
   SM_properties,
   SM_specificity,
   SM_cnt,
   SM_SIZE
};

static int test_scripts = 0;


static int selector_compare(const void *p1, const void *p2)
{
   const Value *v1 = p1;
   const Value *v2 = p2;

   if (v1[SM_specificity].value < v2[SM_specificity].value) {
      return -1;
   }
   if (v1[SM_specificity].value > v2[SM_specificity].value) {
      return +1;
   }
   if (v1[SM_cnt].value < v2[SM_cnt].value) {
      return -1;
   }
   if (v1[SM_cnt].value > v2[SM_cnt].value) {
      return +1;
   }
   return 0;
}


static Value sort_current_selectors(Heap *heap, Value *error, int num_params, Value *params, void *data)
{
   Value *values;
   int err, len;

   err = fixscript_get_array_length(heap, params[0], &len);
   if (err) {
      return fixscript_error(heap, error, err);
   }

   if (len > 1000000*SM_SIZE || (len % SM_SIZE) != 0) {
      *error = fixscript_create_string(heap, "invalid selector array", -1);
      return fixscript_int(0);
   }

   values = malloc(len * sizeof(Value));
   if (!values) {
      return fixscript_error(heap, error, FIXSCRIPT_ERR_OUT_OF_MEMORY);
   }

   err = fixscript_get_array_range(heap, params[0], 0, len, values);
   if (err) {
      free(values);
      return fixscript_error(heap, error, err);
   }

   qsort(values, len / SM_SIZE, SM_SIZE * sizeof(Value), selector_compare);

   err = fixscript_set_array_range(heap, params[0], 0, len, values);
   free(values);
   if (err) {
      return fixscript_error(heap, error, err);
   }
   
   return fixscript_int(0);
}


static Script *load_script(Heap *heap, const char *fname, Value *error, void *data)
{
   if (test_scripts) {
      return fixscript_load_file(heap, fname, error, ".");
   }
   else {
      return fixscript_load_embed(heap, fname, error, embed_scripts);
   }
}


static Heap *create_heap(void *data)
{
   Heap *heap;

   heap = fixscript_create_heap();
   fixio_register_functions(heap);
   fixtask_register_functions(heap, create_heap, NULL, load_script, NULL);
   register_bigint_functions(heap);
   register_crypto_functions(heap);
   register_util_functions(heap);
   register_script_functions(heap);
   //register_image_functions(heap);
   register_css_functions(heap);
   fixscript_register_native_func(heap, "resource_get#1", embed_resources_proxy_get_func, NULL);
   fixscript_register_native_func(heap, "sort_current_selectors#1", sort_current_selectors, NULL);
   return heap;
}


int main(int argc, char **argv)
{
   Heap *heap;
   Script *script;
   Value error;
   int port = 8080;
   int i;

#ifdef _WIN32
   init_critical_sections();
#else
   signal(SIGPIPE, SIG_IGN);
#endif

   start_global_cleanup_thread();
   
   for (i=1; i<argc; i++) {
      if (strcmp(argv[i], "-t") == 0) {
         test_scripts = 1;
      }
      else if (strcmp(argv[i], "-p") == 0) {
         if (++i >= argc) {
            fprintf(stderr, "error: expecting port number\n");
            return 1;
         }
         if (sscanf(argv[i], "%d", &port) != 1) {
            fprintf(stderr, "error: invalid port number\n");
            return 1;
         }
         if (port < 1 || port > 65535) {
            fprintf(stderr, "error: invalid port number\n");
            return 1;
         }
      }
   }

   heap = create_heap(NULL);

   script = load_script(heap, "proxy/main", &error, NULL);
   if (!script) {
      fprintf(stderr, "%s\n", fixscript_get_compiler_error(heap, error));
      fflush(stderr);
      return 0;
   }
   
   fixscript_run(heap, script, "main#1", &error, fixscript_int(port));
   if (error.value) {
      fixscript_dump_value(heap, error, 1);
      return 0;
   }

   return 1;
}
