/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2023 Matus & Michal Tomlein

 Leaklog is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public Licence
 as published by the Free Software Foundation; either version 2
 of the Licence, or (at your option) any later version.

 Leaklog is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public Licence for more details.

 You should have received a copy of the GNU General Public Licence
 along with Leaklog; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
********************************************************************/

/*
 Original code by Angel Marin, Paul Johnston.
*/

#include "sha256.h"

#include <QString>
#include <QByteArray>
#include <QVector>

QVector<quint32> byteArrayToBinb(const QByteArray &bytes)
{
    quint32 mask = (1 << 8) - 1;
    quint32 length = bytes.length() * 8;
    QVector<quint32> bin(((length - 1) >> 5) + 1);
    for (quint32 i = 0; i < length; i += 8) {
        bin[i >> 5] |= (bytes.at(i / 8) & mask) << (32 - 8 - i % 32);
    }
    return bin;
}

QString binbToHex(const QVector<quint32> & binarray)
{
    QString hex_tab = "0123456789abcdef";
    QString str;
    quint32 length = binarray.size() * 4;
    for (quint32 i = 0; i < length; i++) {
        str += hex_tab.at((binarray[i >> 2] >> ((3 - i % 4) * 8 + 4)) & 0xF);
        str += hex_tab.at((binarray[i >> 2] >> ((3 - i % 4) * 8)) & 0xF);
    }
    return str;
}

inline quint32 rotr(quint32 x, int n)
{
    if (n < 32) return (x >> n) | (x << (32 - n));
    return x;
}

inline quint32 ch(quint32 x, quint32 y, quint32 z)
{
    return (x & y) ^ (~x & z);
}

inline quint32 maj(quint32 x, quint32 y, quint32 z)
{
    return (x & y) ^ (x & z) ^ (y & z);
}

inline quint32 sigma0(quint32 x)
{
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

inline quint32 sigma1(quint32 x)
{
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

inline quint32 gamma0(quint32 x)
{
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

inline quint32 gamma1(quint32 x)
{
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

QVector<quint32> sha256core(QVector<quint32> message, quint32 source_binlength)
{
    quint32 K[] = { 0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5, 0x3956C25B,
                    0x59F111F1, 0x923F82A4, 0xAB1C5ED5, 0xD807AA98, 0x12835B01,
                    0x243185BE, 0x550C7DC3, 0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7,
                    0xC19BF174, 0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC,
                    0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA, 0x983E5152,
                    0xA831C66D, 0xB00327C8, 0xBF597FC7, 0xC6E00BF3, 0xD5A79147,
                    0x06CA6351, 0x14292967, 0x27B70A85, 0x2E1B2138, 0x4D2C6DFC,
                    0x53380D13, 0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
                    0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3, 0xD192E819,
                    0xD6990624, 0xF40E3585, 0x106AA070, 0x19A4C116, 0x1E376C08,
                    0x2748774C, 0x34B0BCB5, 0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F,
                    0x682E6FF3, 0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
                    0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2 };
    QVector<quint32> H(8);
    H[0] = 0x6A09E667; H[1] = 0xBB67AE85; H[2] = 0x3C6EF372; H[3] = 0xA54FF53A;
    H[4] = 0x510E527F; H[5] = 0x9B05688C; H[6] = 0x1F83D9AB; H[7] = 0x5BE0CD19;
    QVector<quint32> W(64);
    quint32 a, b, c, d, e, f, g, h;
    quint32 T1, T2;
    int new_size = (((source_binlength + 1 + 64) >> 9) << 4) + 16;
    message.resize(new_size);
    message[source_binlength >> 5] |= 0x80 << (24 - source_binlength % 32);
    message[new_size - 1] = source_binlength;
    int message_length = message.size();
    for (int i = 0; i < message_length; i += 16) {
        a = H[0]; b = H[1]; c = H[2]; d = H[3]; e = H[4]; f = H[5]; g = H[6]; h = H[7];
        for (quint32 t = 0; t < 64; t++) {
            if (t < 16) W[t] = message[t + i];
            else W[t] = gamma1(W[t - 2]) + W[t - 7] + gamma0(W[t - 15]) + W[t - 16];
            T1 = h + sigma1(e) + ch(e, f, g) + K[t] + W[t];
            T2 = sigma0(a) + maj(a, b, c);
            h = g; g = f; f = e; e = d + T1; d = c; c = b; b = a; a = T1 + T2;
        }
        H[0] += a; H[1] += b; H[2] += c; H[3] += d; H[4] += e; H[5] += f; H[6] += g; H[7] += h;
    }
    return H;
}

QString sha256(const QString &source)
{
    QByteArray bytes = source.toUtf8();
    return binbToHex(sha256core(byteArrayToBinb(bytes), bytes.length() * 8));
}
