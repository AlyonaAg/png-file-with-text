#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <png.h>
#include <stdint.h>
#include <stdlib.h>

FT_Glyph getGlyph(FT_Face face, uint32_t charcode);
FT_Pos getKerning(FT_Face face, uint32_t leftCharcode, uint32_t rightCharcode);
void savePNG(uint8_t* image, int32_t width, int32_t height, char* image_file);

#ifdef DYNAMIC
#include <dlfcn.h>
int (*Init_FreeType)(FT_Library*);
int (*New_Face)(FT_Library, const char*, FT_Long, FT_Face*);
int (*Set_Pixel_Sizes)(FT_Face, FT_UInt, FT_UInt);
int (*Done_Glyph)(FT_Glyph);
int (*Load_Char)(FT_Face, FT_ULong, FT_Int32);
int (*Get_Glyph)(FT_GlyphSlot, FT_Glyph*);
int (*Get_Char_Index)(FT_Face, FT_ULong);
int (*Get_Kerning)(FT_Face, FT_UInt, FT_UInt, FT_UInt, FT_Vector*);
int (*Done_Face)(FT_Face);
int (*Done_FreeType)(FT_Library);

void (*init_io)(png_structrp, png_FILE_p);
void (*write_end)(png_structrp, png_inforp);
void (*write_row)(png_structrp, png_const_bytep);
void (*write_info)(png_structrp, png_const_inforp);
void* (*create_info_struct)(png_const_structrp);
void (*destroy_write_struct)(png_structpp, png_infopp);
void* (*create_write_struct)(png_const_charp, png_voidp, png_error_ptr, png_error_ptr);
void (*set_IHDR)(png_const_structrp, png_inforp, png_uint_32, png_uint_32, int, int, int, int, int);
void (*free_data)(png_const_structrp, png_inforp, png_uint_32, int);


#define FT_New_Face New_Face
#define FT_Load_Char Load_Char
#define FT_Get_Glyph Get_Glyph
#define FT_Done_Glyph Done_Glyph
#define FT_Init_FreeType Init_FreeType
#define FT_Get_Char_Index Get_Char_Index
#define FT_Set_Pixel_Sizes Set_Pixel_Sizes
#define FT_Get_Kerning Get_Kerning
#define FT_Done_Face Done_Face
#define FT_Done_FreeType Done_FreeType

#define png_init_io init_io
#define png_set_IHDR set_IHDR
#define png_write_end write_end
#define png_write_row write_row
#define png_write_info write_info
#define png_create_info_struct create_info_struct
#define png_create_write_struct create_write_struct
#define png_destroy_write_struct destroy_write_struct
#define png_free_data free_data

void* DL_freetype;
void* DL_libpng;

int DYNLIB_open()
{
    DL_freetype = dlopen("./libfreetype.so", RTLD_LAZY);
    if (!DL_freetype)
    {
        printf("Bad load libfreetype.so\n");
        return 0;
    }
    DL_libpng = dlopen("./libpng.so", RTLD_LAZY);
    if (!DL_libpng)
    {
        printf("Bad load libpng.so\n");
        return 0;
    }

    New_Face = dlsym(DL_freetype, "FT_New_Face");
    Get_Glyph = dlsym(DL_freetype, "FT_Get_Glyph");
    Load_Char = dlsym(DL_freetype, "FT_Load_Char");
    Done_Glyph = dlsym(DL_freetype, "FT_Done_Glyph");
    Init_FreeType = dlsym(DL_freetype, "FT_Init_FreeType");
    Get_Char_Index = dlsym(DL_freetype, "FT_Get_Char_Index");
    Set_Pixel_Sizes = dlsym(DL_freetype, "FT_Set_Pixel_Sizes");
    Get_Kerning = dlsym(DL_freetype, "FT_Get_Kerning");
    Done_Face = dlsym(DL_freetype, "FT_Done_Face");
    Done_FreeType = dlsym(DL_freetype, "FT_Done_FreeType");

    init_io = dlsym(DL_libpng, "png_init_io");
    set_IHDR = dlsym(DL_libpng, "png_set_IHDR");
    write_end = dlsym(DL_libpng, "png_write_end");
    create_info_struct = dlsym(DL_libpng, "png_create_info_struct");
    create_write_struct = dlsym(DL_libpng, "png_create_write_struct");
    destroy_write_struct = dlsym(DL_libpng, "png_destroy_write_struct");
    write_row = dlsym(DL_libpng, "png_write_row");
    write_info = dlsym(DL_libpng, "png_write_info");
    free_data = dlsym(DL_libpng, "png_free_data");

    return 1;
}

int DYNLIB_close()
{
    dlclose(DL_freetype);
    dlclose(DL_libpng);
}
#endif

// Позиция и размер глифа в строке
struct Symbol
{ 
    int32_t posX;               // Позиция по горизонтали
    int32_t posY;               // Позиция по вертикали (от базовой линии)
    int32_t width;              // Ширина глифа
    int32_t height;             // Высота глифа

    FT_Glyph glyph;
};

const size_t MAX_SYMBOLS_COUNT = 128;

#define MIN(x, y) ((x) > (y) ? (y) : (x))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

/*
 Первый параметр - *ttf
 Второй параметр - текст
 Третий параметр - *png
*/
#ifdef BLOB

void** functions_global;

clock_t clock(void)
{
    return ((clock_t(*)(void))functions_global[0])();
}

FILE* fopen(const char* fname, const char* mode)
{
    return ((FILE * (*)(const char*, const char*))functions_global[1])(fname, mode);
}

void* malloc(size_t size)
{
    return ((void* (*)(size_t))functions_global[3])(size);
}

void free(void* ptr)
{
    return ((void (*)(void*))functions_global[4])(ptr);
}

size_t strlen(const char* str)
{
    return ((size_t(*)(const char*))functions_global[5])(str);
}

void* mmap(void* start, size_t length, int prot, int flags, int fd, off_t offset) {
    return ((char* (*)(void*, size_t, int, int, int, off_t))functions_global[6])(start, length, prot, flags, fd, offset);
}

int main(int argc, char* argv[], void** functions)
#else
int main(int argc, char* argv[])
#endif
{
#ifdef BLOB
    functions_global = functions;
#endif
    clock_t start = clock();

    if (argc != 4)
    {
    
        printf("Input parameters are set incorrectly. Input parameters format: [font file *.ttf] [text] [image file *.png].\n");
        return 1;
    }

    char* font = argv[1];

    FILE* file_font = fopen(font, "rb");
    if (!file_font)
    {
        printf("Font file could not be opened.\n");
        return 1;
    }
    fclose(file_font);
    
#ifdef DYNAMIC
    if (!DYNLIB_open())
    {
        //printf("Open lib error.\n");
        return 1;
    }
    printf("System boot time: %ld\n", (clock() - start));
#endif

    // Инициализация библиотеки
    FT_Library ftLibrary = 0;
    FT_Init_FreeType(&ftLibrary);

    // Загрузка шрифта из текущей папки
    FT_Face ftFace = 0;
    FT_New_Face(ftLibrary, argv[1], 0, &ftFace);

    // Установка размера символа
    FT_Set_Pixel_Sizes(ftFace, 100, 0);

    const char* exampleString = argv[2];                        // Выводимая строка
    const size_t exampleStringLen = strlen(exampleString);

    // Набор готовых символов
    struct Symbol symbols[MAX_SYMBOLS_COUNT];
    size_t numSymbols = 0;
    int32_t left = INT_MAX;
    int32_t top = INT_MAX;
    int32_t bottom = INT_MIN;
    uint32_t prevCharcode = 0;

    size_t i = 0;

    // Позиция текущего символа в формате 26.6
    int32_t posX = 0;

    for (i = 0; i < exampleStringLen; ++i)
    {
        // Получение кода символа
        const uint32_t charcode = exampleString[i];

        // Получение глифа для этого символа
        FT_Glyph glyph = getGlyph(ftFace, charcode);

        if (!glyph)
        {
            // Глифы в шрифте есть не для всех символов
            continue;
        }

        if (prevCharcode)
        {
            // Используем кернинг
            posX += getKerning(ftFace, prevCharcode, charcode);
        }
        prevCharcode = charcode;

        struct Symbol* symb = &(symbols[numSymbols++]);

        FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)glyph;
        symb->posX = (posX >> 6) + bitmapGlyph->left;           // Вычисление горизонтальной позиции символа
        symb->posY = -bitmapGlyph->top;                         // Вычисление вертикальной позиции символа относительно базовой линии. Отрицательные значение - сверху, полоэительные - снизу
        symb->width = bitmapGlyph->bitmap.width;                // Ширина символа
        symb->height = bitmapGlyph->bitmap.rows;                // Высота символа
        symb->glyph = glyph;                                    // ССылка на глиф 

        posX += glyph->advance.x >> 10;                         // Смещаем позицию текущего символа (glyph->advance имеет формат 16.16, а мы хотим 26.6)

        left = MIN(left, symb->posX);                           // Вычисление самой левой позиции
        top = MIN(top, symb->posY);                             // Вычисление самой верхней позиции
        bottom = MAX(bottom, symb->posY + symb->height);        // Вычисление самой нижней позиции
    }

    for (i = 0; i < numSymbols; ++i)
    {
        symbols[i].posX -= left;                                // Смещение всех символов влево, чтобы строка привыкала к левой части
    }

    const struct Symbol* lastSymbol = &(symbols[numSymbols - 1]);

    const int32_t imageW = lastSymbol->posX + lastSymbol->width;    // Ширина строки (самая крайняя права точка последнего символа в конце)
    const int32_t imageH = bottom - top;                            // Высота строки (изображения)

    uint8_t* image = malloc(imageW * imageH);                   // Выделение памяти под картинку

    for (i = 0; i < numSymbols; ++i)
    {
        const struct Symbol* symb = symbols + i;

        FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)symb->glyph;
        FT_Bitmap bitmap = bitmapGlyph->bitmap;

        for (int32_t srcY = 0; srcY < symb->height; ++srcY)
        {
            const int32_t dstY = symb->posY + srcY - top;       // Координата Y в итоговой картинке

            for (int32_t srcX = 0; srcX < symb->width; ++srcX)
            {
                // Получение пикселя из изображения символа
                const uint8_t c = bitmap.buffer[srcX + srcY * bitmap.pitch];

                if (0 == c)
                {
                    // Пропускание полностью прозрачных пикселей
                    continue;
                }

                const float a = c / 255.0f;                     // Приведение множества [0..255] к [0..1] для удобсва блендинга
                const int32_t dstX = symb->posX + srcX;         // Координата X в итоговой картинке
                uint8_t* dst = image + dstX + dstY * imageW;    // Вычисление смещения в итоговой картинке
                dst[0] = (uint8_t)(a * 255 + (1 - a) * dst[0]); // Рисуем этот пиксель в итоговую картинку с блендингом
            }
        }
    }
    
    savePNG(image, imageW, imageH, argv[3]);                    // Сохранение изображения в PNG формате

    free(image);

    for (i = 0; i < numSymbols; ++i)
    {
        // Особождение памяти для глифов
        FT_Done_Glyph(symbols[i].glyph);
    }

    // Освобождение шрифта
    FT_Done_Face(ftFace);
    ftFace = 0;

    // Окончание работы с библиотекой
    FT_Done_FreeType(ftLibrary);
    ftLibrary = 0;
    printf("Total running time: %ld\n", (clock() - start));
    return 0;
}


FT_Glyph getGlyph(FT_Face face, uint32_t charcode)
{
    FT_Load_Char(face, charcode, FT_LOAD_RENDER);       // Загрузка глифа в face->glyph с отрисовкой
    FT_Glyph glyph = 0;
    FT_Get_Glyph(face->glyph, &glyph);                  // Получение глифа
    return glyph;
}


FT_Pos getKerning(FT_Face face, uint32_t leftCharcode, uint32_t rightCharcode)
{
    FT_UInt leftIndex = FT_Get_Char_Index(face, leftCharcode);      // Получение индекса левого символа
    FT_UInt rightIndex = FT_Get_Char_Index(face, rightCharcode);    // Получение индекса правого символа

    FT_Vector delta;            // Здесь кернинг в формате 26.6
    FT_Get_Kerning(face, leftIndex, rightIndex, FT_KERNING_DEFAULT, &delta);    // Получение кернинга для двух символов
    return delta.x;
}


void savePNG(uint8_t* image, int32_t width, int32_t height, char* image_file)
{
    FILE* f = fopen(image_file, "wb");              // Открытие файла для сохранения картинки

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

    png_infop info_ptr = png_create_info_struct(png_ptr);

    png_init_io(png_ptr, f);

    // Изображение в формате RGBA по 8 бит на канал и по четыре канала на пиксель
    png_set_IHDR(
        png_ptr,
        info_ptr,
        width,
        height,
        8,
        PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

    uint8_t* row = malloc(width * 4);       // Одна строка в формате RGBA, 4 канала

    // Сохранение в PNG построчно
    for (int32_t y = 0; y < height; ++y)
    {
        // Преобразование строки из одноканальной в RGBA
        for (int32_t x = 0; x < width; ++x)
        {
            // Цвет изображения
            row[x * 4 + 0] = 0x00;
            row[x * 4 + 1] = 0x00;
            row[x * 4 + 2] = 0x00;
            // Прозрачность берём из исходный данных
            row[x * 4 + 3] = image[y * width + x];
        }
        // Сохранение строки в PNG
        png_write_row(png_ptr, row);
    }

    free(row);

    png_write_end(png_ptr, 0);

    // Закончили сохранение и освобождаем ресурсы
    png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    png_destroy_write_struct(&png_ptr, 0);
    fclose(f);
}