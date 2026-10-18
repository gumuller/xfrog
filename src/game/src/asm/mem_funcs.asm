;Xfrog - a frogger clone.
;Copyright (C) 2000 Gustavo S. Muller
;
;This program is free software; you can redistribute it and/or
;modify it under the terms of the GNU General Public License
;as published by the Free Software Foundation; either version 2
;of the License, or (at your option) any later version.
;
;This program is distributed in the hope that it will be useful,
;but WITHOUT ANY WARRANTY; without even the implied warranty of
;MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;GNU General Public License for more details.
;
;You should have received a copy of the GNU General Public License
;along with this program; if not, write to the Free Software
;Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

;**************************************************************
;
; Author:		Gustavo Simi Muller -> muller@netpar.com.br
;
; Date:			24/01/2000
;
; Description:  New memsets and memcpys
;
;**************************************************************


[BITS 32]


[SECTION .text align=32]


; Prototype: void dword_memset(void* dest, int32 num, int32 size);
; Fills memory with 4 bytes per loop iteration

[GLOBAL _dword_memset]


_dword_memset:
				push	edi


				mov		edi, [esp+8]			; dest
				mov		eax, [esp+12]			; num
				mov		ecx, [esp+16]			; size

.l1
				mov		[edi], eax
				add		edi, byte 4
				dec		ecx
				jnz		.l1

				pop		edi
				ret


; Prototype: void dword_memcpy(void* dest, void* src, int32 count);
; Copies memory using DWORDs

[GLOBAL _dword_memcpy]


_dword_memcpy:
				push	esi
				push	edi

				mov		edi, [esp+12]			; dest
				mov		esi, [esp+16]			; src
				mov		ecx, [esp+20]			; count

.l2
				mov		eax, [esi]
				add		esi, byte 4
				mov		[edi], eax
				add		edi, byte 4

				dec		ecx
				jnz		.l2

				pop		edi
				pop		esi
				ret