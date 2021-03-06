﻿//
// pch.h
// Заголовок стандартных системных включаемых файлов.
//

#pragma once

#include <collection.h>
#include <ppltasks.h>
#include <xaudio2.h>

#include "App.xaml.h"

// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr);

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str);
