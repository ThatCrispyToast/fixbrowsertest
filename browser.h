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

#ifndef BROWSER_H
#define BROWSER_H

#include "fixscript.h"

enum {
   HANDLE_TYPE_AES_STATE,
   HANDLE_TYPE_SCRIPT_HEAP,
   HANDLE_TYPE_SCRIPT_VALUE
};

void register_bigint_functions(Heap *heap);
void register_crypto_functions(Heap *heap);
void register_script_functions(Heap *heap);
void register_image_functions(Heap *heap);
void register_css_functions(Heap *heap);

#endif /* BROWSER_H */
