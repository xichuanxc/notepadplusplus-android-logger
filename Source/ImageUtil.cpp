/*
  AndroidLogger: Android Logger plugin for Notepad++
  Copyright (C) 2015 Simbaba at Najing <zhaoxi.du@gmail.com>

  ******************************************************
  Thanks for GedcomLexer & NppFtp plugin source code.
  ******************************************************
 
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/
#include "ImageUtil.h"
#include <iostream>
#include <fstream>


int getMask(int length) {
    int res = 0;

    for (int i = 0 ; i < length ; i++) {
        res = (res << 1) + 1;
    }
    
    return res;
}

//����BMPλͼ�ļ�ͷ
void ContructBhh(int nWidth,int nHeight,BITMAPFILEHEADER& bhh)
{
    //ÿ��ʵ��ռ�õĴ�С��ÿ�ж�����䵽һ��4�ֽڱ߽磩
    int widthStep = (((nWidth * 24) + 31) & (~31)) / 8 ;

    bhh.bfType = ((WORD) ('M' << 8) | 'B');  //'BM'
    bhh.bfSize = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + widthStep * nHeight;
    bhh.bfReserved1 = 0;
    bhh.bfReserved2 = 0;
    bhh.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
}

//����BMP�ļ���Ϣͷ
void ConstructBih(int nWidth,int nHeight,BITMAPINFOHEADER& bih)
{
    int widthStep = (((nWidth * 24) + 31) & (~31)) / 8 ;

    bih.biSize=40;       // header size
    bih.biWidth=nWidth;
    bih.biHeight=nHeight;
    bih.biPlanes=1;
    bih.biBitCount=24;     // RGB encoded, 24 bit
    bih.biCompression=BI_RGB;   // no compression ��ѹ��
    bih.biSizeImage=widthStep*(nHeight>0?nHeight:-nHeight)*3;
    bih.biXPelsPerMeter=0;
    bih.biYPelsPerMeter=0;
    bih.biClrUsed=0;
    bih.biClrImportant=0;
}

BYTE* RawImage16toARGB(raw_image_t *raw)
{
    BYTE *buffer = new BYTE[3*raw->width * raw->height];
    int index = 0;

    for (int y = 0 ; y < raw->height ; y++) {
        for (int x = 0 ; x < raw->width ; x++) {
            int value = raw->data[index++] & 0x00FF;
            value |= (raw->data[index++] << 8) & 0x0FF00;

            int r = ((value >> 11) & 0x01F) << 3;
            int g = ((value >> 5) & 0x03F) << 2;
            int b = ((value     ) & 0x01F) << 3;

            //scanline[x] = 0xFF << 24 | r << 16 | g << 8 | b;
            buffer[y * raw->width * 3 + x*3] = b;
            buffer[y * raw->width * 3 + x*3 + 1] = g;
            buffer[y * raw->width * 3 + x*3 + 2] = r;
        }
    }

    return buffer;
}

BYTE* RawImage32toARGB(raw_image_t *raw)
{
    BYTE *buffer = new BYTE[3*raw->width * raw->height];
    int index = 0;

    int redOffset = raw->red_offset;
    int redLength = raw->red_length;
    int redMask = getMask(redLength);
    int greenOffset = raw->green_offset;
    int greenLength = raw->green_length;
    int greenMask = getMask(greenLength);
    int blueOffset = raw->blue_offset;
    int blueLength = raw->blue_length;
    int blueMask = getMask(blueLength);
    int alphaLength = raw->alpha_length;
    int alphaOffset = raw->alpha_offset;
    int alphaMask = getMask(alphaLength);

    for (int y = 0 ; y < raw->height ; y++) {
        for (int x = 0 ; x < raw->width ; x++) {
            int value = raw->data[index++] & 0x00FF;
            value |= (raw->data[index++] & 0x00FF) << 8;
            value |= (raw->data[index++] & 0x00FF) << 16;
            value |= (raw->data[index++] & 0x00FF) << 24;

            unsigned uValue = (unsigned)value;

            int r = ((uValue >> redOffset) & redMask) << (8 - redLength);
            int g = ((uValue >> greenOffset) & greenMask) << (8 - greenLength);
            int b = ((uValue >> blueOffset) & blueMask) << (8 - blueLength);
            int a = 0xFF;

            if (alphaLength != 0) {
                a = ((uValue >> alphaOffset) & alphaMask) << (8 - alphaLength);
            }

            ///scanline[x] = a << 24 | r << 16 | g << 8 | b;
            buffer[y * raw->width * 3 + x*3] = b;
            buffer[y * raw->width * 3 + x*3 + 1] = g;
            buffer[y * raw->width * 3 + x*3 + 2] = r;
        }
    }
    return buffer;
}

BYTE* Convert2RGB888(raw_image_t *raw)
{
     BYTE *buffer = new BYTE[3*raw->width * raw->height];

    switch(raw->bpp)
    {
    case 16:
        return RawImage16toARGB(raw);
    case 32:
        return RawImage32toARGB(raw);
    default:
        return NULL;
    }

    return buffer;
}

BOOL SaveDIB2Bmp(BYTE *data, int iWidth, int iHeight, LPCTSTR filePath)
{
    BITMAPINFOHEADER bih;
    ConstructBih(iWidth,-iHeight,bih);

    BITMAPFILEHEADER bhh;
    ContructBhh(iWidth,iHeight,bhh);

    int widthStep = (((iWidth * 24) + 31) & (~31)) >> 3;
    int DIBSize = widthStep * iHeight ;

    std::ofstream of(filePath, std::ios::binary);
    of.write((LPSTR)&bhh, sizeof(BITMAPFILEHEADER));
    of.write((LPSTR)&bih, sizeof(BITMAPINFOHEADER));
    of.write((char*)data, DIBSize);
    of.close();

    return TRUE;
}

//�ú�������sin(x) / (x)   
float Sinxx(float fx)   
{   
    float fabsx = abs(fx);   
    float fxx = fabsx * fabsx;   
    float fxxx = fxx * fabsx;   
    if(fabsx < 1.0f) return (1.0f - 2.0f * fxx + fxxx);   
    else if(fabsx < 2.0f)    
        return (4.0f - 8.0f * fabsx + 5.0f * fxx - fxxx);    
    else return 0.0f;   
} 


HBITMAP CreateDdb(HDC hDC, int bitCount, int nWidth, int nHeight, const LPBYTE lpbyBits)
{
    //��һ��, ����һ��BITMAPV4HEADER�ṹ   
   
    //�����ڴ�:   
    BYTE* pbyBits = new BYTE[sizeof(BITMAPV4HEADER)];   
    if(pbyBits == NULL) return NULL;

    //ָ��Dibʵ���������ݵ�ָ��,    
    BITMAPV4HEADER* pbmih = (BITMAPV4HEADER*)pbyBits;   
   
    pbmih->bV4Size = sizeof(BITMAPV4HEADER); //108   
    pbmih->bV4Width = (LONG)nWidth;   
    pbmih->bV4Height = (LONG)(-nHeight);   
    pbmih->bV4Planes = 1;   
    pbmih->bV4BitCount = bitCount << 3;   
   
    pbmih->bV4V4Compression = BI_RGB;       
    pbmih->bV4SizeImage = 0;   
    pbmih->bV4XPelsPerMeter = 0;       
    pbmih->bV4YPelsPerMeter = 0;   
    pbmih->bV4ClrUsed = 0;       
    pbmih->bV4ClrImportant = 0;   
    
    //�ڶ���, ����һ��DDB   
    HBITMAP hBitmap = ::CreateDIBitmap(hDC, (BITMAPINFOHEADER*)pbmih, CBM_INIT, lpbyBits, (BITMAPINFO*)pbmih, DIB_RGB_COLORS);   
   
    delete[] pbyBits;   
    return hBitmap;   
}

BOOL CImageUtil::Scale(LPBYTE lpbyBitsSrc, int nWidth, int nHeight, LPBYTE lpbyBitsDst, int nWidthImgDst, int nHeightImgDst)
{
    return Scale(lpbyBitsSrc, 0, 0, nWidth, nHeight, nWidth, nHeight, lpbyBitsDst, nWidthImgDst, nHeightImgDst);
}

BOOL CImageUtil::Scale(LPBYTE lpbyBitsSrc, int x, int y, int nWidth, int nHeight, int nScanWidth, int nScanHeight, LPBYTE lpbyBitsDst, int nWidthImgDst, int nHeightImgDst)
{
     //��һ��, ���в����Ϸ��Լ��   
    //ASSERT(lpbyBitsSrc32);   
    //ASSERT(lpbyBitsDst32);   
    //ASSERT(nWidthImgDst != 0);   
    //ASSERT(nHeightImgDst != 0);   
   
    if((x > (nScanWidth - 1)) || (y > (nScanHeight - 1))) return FALSE;   


    //��Ч����Ŀ�Ⱥ͸߶�   
    int w = min(nWidth, nScanWidth - x);   
    int h = min(nHeight, nScanHeight - y);   
       
   
    //ע������:   
    //��һ:   
    //���(w <  nWidth), ����(h <  nHeight)���ʾָ��������ܹ���Ч��ȡ���ݵ�����Ҫ��,    
    //��ʱ���򽫷Ŵ���, ʹ������ŵĽ�����ܴﵽ nWidthImgDst ��� nHeightImgDst ��   
       
    //�ڶ�:      
    //fScalex, fScaley����ʾ�����ű�Ϊ��ʵ���űȵĵ���   
    //֮������������������, ��һ�γ���, �ܱ�һ�γ˷�Ҫ��.   
   
    //������ű�   
    float fScalex = (float)w / (float)nWidthImgDst;   
    float fScaley = (float)h / (float)nHeightImgDst;   
   
    //���ֽ���   
    DWORD dwWidthBytes = (DWORD)nScanWidth * m_bitCount;   
    //��ʼ���ݻ�����   
    DWORD dwBaseIndex = y * dwWidthBytes + m_bitCount * x;   
   
    //ָ��Ŀ������   
    BYTE* pbyDst = lpbyBitsDst;

    //��ɱ任   
    for(int i = 0; i < nHeightImgDst;i++)
    {   
        //����任���õĸ���yֵ
        float fYInverse = i * fScaley;
        //ȡ��   
        int yy = (int)fYInverse;
   
        //�����ֵ   
        float fv = fYInverse - yy;
           
        //��Ӧ��ԭͼ���y����
        yy += y;   
   
        BYTE* pbySrc = lpbyBitsSrc + yy * dwWidthBytes;
 
        for(int j = 0;j < nWidthImgDst;j++)
        {   
            //����任���õĸ���xֵ
            float fXInverse = j * fScalex;
            //ȡ��   
            int xx = (int)fXInverse;
  
            //�����ֵ
            float fu = fXInverse - xx;

            //��Ӧ��ԭͼ���y����
            xx += x;   
 
            //��ȡ����
            BYTE* pbyCurrent =  pbySrc + m_bitCount * xx;
            PIXELCOLORRGB rgb = Interpolate(pbyCurrent, xx, yy, fu, fv, nScanWidth,  nScanHeight);
               
            *pbyDst++ = rgb.blue;
            *pbyDst++ = rgb.green;
            *pbyDst++ = rgb.red;
       
            //alpha�������ⲿ���.
            if ( m_bitCount == 4 ) {
                pbyDst++;
            }
        }
    }
    return TRUE;
}


PIXELCOLORRGB CImageUtil::Interpolate(LPBYTE lpbySrcXY, int x, int y, float fu, float fv, int nScanWidth, int nScanHeight)
{
    PIXELCOLORRGB rgb;
 
    //���ֽ���, ���Խ�dwWidthBytes��Ϊ�������ݹ���
    DWORD dwWidthBytes = (DWORD)nScanWidth * m_bitCount;
       
    switch(m_dwQuality)
    {   
        case IMAGE_NEAREST_NEIGHBOR_INTERPOLATE:
        {   
            BYTE* pbySrc = lpbySrcXY;
            rgb.blue = *pbySrc++;
            rgb.green = *pbySrc++;
            rgb.red = *pbySrc++;
            break;
        }   
        case IMAGE_BILINEAR_INTERPOLATE:
        {   
            //���ڵ��ĸ����������½ǵ��x, y����ƫ����
            int nx = 1;
            int ny = 1;
            if((x + 1) > (nScanWidth - 1)) nx = 0;
            if((y + 1) > (nScanHeight - 1)) ny = 0;
 
            //�����ĸ����ص�����ֵ
            BYTE abyRed[2][2], abyGreen[2][2], abyBlue[2][2];
  
            //���ص�(x, y)������λ��
            BYTE* pbySrc = lpbySrcXY;
            //��ȡ������ֵ.
            //(x, y) = (x, y) + (0, 0)   
            abyBlue[0][0] = *pbySrc++;   
            abyGreen[0][0] = *pbySrc++;   
            abyRed[0][0] = *pbySrc++;   
               
            //(x + 1, y) = (x, y) + (1, 0)   
            pbySrc = (lpbySrcXY + nx * dwWidthBytes);
            abyBlue[1][0] = *pbySrc++;
            abyGreen[1][0] = *pbySrc++;
            abyRed[1][0] = *pbySrc++;
   
               
            //ָ����һ������
            BYTE* pbySrcTemp = (lpbySrcXY + ny * dwWidthBytes);
   
            //(x , y + 1) = (x, y) + (0, 1)
            pbySrc = pbySrcTemp;
            abyBlue[0][1] = *pbySrc++;
            abyGreen[0][1] = *pbySrc++;
            abyRed[0][1] = *pbySrc++;
   
            //(x + 1, y + 1) = (x, y) + (1, 1)
            pbySrc = pbySrcTemp + (m_bitCount * nx);
            abyBlue[1][1] = *pbySrc++;
            abyGreen[1][1] = *pbySrc++;
            abyRed[1][1] = *pbySrc++;
               
            rgb.red = (BYTE)(BOUND(((1 - fu) * (1 - fv) * ((float)abyRed[0][0]) +
                      (1 - fu) * fv * ((float)abyRed[0][1]) +
                      fu * (1 - fv) * ((float)abyRed[1][0]) +
                      fu * fv * ((float)abyRed[1][1])), 0, 255));
            rgb.green = (BYTE)(BOUND(((1 - fu) * (1 - fv) * ((float)abyGreen[0][0]) +
                      (1 - fu) * fv * ((float)abyGreen[0][1]) +
                      fu * (1 - fv) * ((float)abyGreen[1][0]) +
                      fu * fv * ((float)abyGreen[1][1])), 0, 255));
   
            rgb.blue = (BYTE)(BOUND(((1 - fu) * (1 - fv) * ((float)abyBlue[0][0]) +    
                      (1 - fu) * fv * ((float)abyBlue[0][1]) +
                      fu * (1 - fv) * ((float)abyBlue[1][0]) +
                      fu * fv * ((float)abyBlue[1][1])), 0, 255));
            break;
        }

        case IMAGE_THREE_ORDER_INTERPOLATE:
        {   
            //��������   
            int xx[4], yy[4];
            //�����ĸ����ص�����ֵ
            BYTE abyRed[4][4], abyGreen[4][4], abyBlue[4][4];
  
            xx[0] = -1;  xx[1] = 0; xx[2] = 1; xx[3] = 2;
            yy[0] = -1;  yy[1] = 0; yy[2] = 1; yy[3] = 2;
   
            //��֤�Ϸ�   
            if((x - 1) < 0) xx[0] = 0;
            if((x + 1) > (nScanWidth - 1)) xx[2] = 0;
            if((x + 2) > (nScanWidth - 1)) xx[3] = ((xx[2] == 0) ? 0 : 1);
   
            if((y - 1) < 0) yy[0] = 0;
            if((y + 1) > (nScanHeight - 1)) yy[2] = 0;
            if((y + 2) > (nScanHeight - 1)) yy[3] = ((yy[2] == 0) ? 0 : 1);
   
            //���ص�(x, y)������λ��
            //��ȡ����
            int i;
            for(i = 0;i < 4;i++)
            {
                //���ص�(x, y)������λ��
                BYTE* pbySrcBase = lpbySrcXY + yy[i] * dwWidthBytes;
   
                for(int j = 0;j < 4;j++)
                {
                    BYTE* pbySrc = pbySrcBase + m_bitCount * xx[j];
                    abyBlue[i][j] = *pbySrc++;
                    abyGreen[i][j] = *pbySrc++;
                    abyRed[i][j] = *pbySrc++;
                }   
            }   
   
            //u, v����   
            float afu[4], afv[4];
   
            afu[0] = Sinxx(1.0f + fu);
            afu[1] = Sinxx(fu);   
            afu[2] = Sinxx(1.0f - fu);
            afu[3] = Sinxx(2.0f - fu);
  
            afv[0] = Sinxx(1.0f + fv);
            afv[1] = Sinxx(fv);
            afv[2] = Sinxx(1.0f - fv);
            afv[3] = Sinxx(2.0f - fv);
   
            //������������м�ֵ   
            float afRed[4] = {0.0f, 0.0f, 0.0f, 0.0f};
            float afGreen[4] = {0.0f, 0.0f, 0.0f, 0.0f};
            float afBlue[4] = {0.0f, 0.0f, 0.0f, 0.0f};
   
            for(i = 0;i < 4;i++)
            {
                for(int j = 0;j < 4;j++)
                {
                    afRed[i] += afv[j] * abyRed[j][i];
                    afGreen[i] += afv[j] * abyGreen[j][i];
                    afBlue[i] += afv[j] * abyBlue[j][i];             
                }
            }
            rgb.red = (BYTE)(BOUND((afu[0] * afRed[0] + afu[1] * afRed[1] + afu[2] * afRed[2] +    
                        afu[3] * afRed[3]), 0, 255));
            rgb.green = (BYTE)(BOUND((afu[0] * afGreen[0] + afu[1] * afGreen[1] + afu[2] * afGreen[2] +    
                        afu[3] * afGreen[3]), 0, 255));
            rgb.blue = (BYTE)(BOUND((afu[0] * afBlue[0] + afu[1] * afBlue[1] + afu[2] * afBlue[2] +    
                        afu[3] * afBlue[3]), 0, 255));
            break;
        }
        default:break;   
    }//end switch   
   
    return rgb;   
}