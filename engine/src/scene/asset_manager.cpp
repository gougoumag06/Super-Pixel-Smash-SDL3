/*
    Copyright (c) Arnaud BANNIER and Nicolas BODIN.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#include "scene/asset_manager.h"
#include "game_engine_common.h"
#include "utils/utils.h"

#ifndef AssertNew
#define AssertNew(ptr) { if (ptr == NULL) { assert(false); abort(); } }
#endif

AssetManager::AssetManager()
    : m_sheetMap()
    , m_fontMap()
    , m_soundMap()
    , m_musicMap()
    , m_textureMap()
    , m_sfxChannels()
    , m_sfxIndex(0)
{
    for (int i = 4; i < 8; i++)
    {
        m_sfxChannels.push_back(i);
    }
}

AssetManager::~AssetManager()
{
    for (auto it = m_textureMap.begin(); it != m_textureMap.end(); ++it)
    {
        TextureData *textureData = it->second;
        if (textureData) delete textureData;
    }
    for (auto it = m_sheetMap.begin(); it != m_sheetMap.end(); ++it)
    {
        SheetData *sheetData = it->second;
        if (sheetData) delete sheetData;
    }
    for (auto it = m_fontMap.begin(); it != m_fontMap.end(); ++it)
    {
        FontData *fontData = it->second;
        if (fontData) delete fontData;
    }
    for (auto it = m_soundMap.begin(); it != m_soundMap.end(); ++it)
    {
        SoundData *soundData = it->second;
        if (soundData) delete soundData;
    }
    for (auto it = m_musicMap.begin(); it != m_musicMap.end(); ++it)
    {
        MusicData *musicData = it->second;
        if (musicData) delete musicData;
    }
}

void AssetManager::AddSpriteSheet(int sheetID, const std::string &assetsPath, const std::string &fileName, bool pixelArt)
{
    const std::string path = assetsPath + fileName;
    auto it = m_sheetMap.find(sheetID);
    if (it != m_sheetMap.end())
    {
        std::cout
            << "ERROR - Add sprite sheet " << path << std::endl
            << "      - The sprite sheet already exists" << std::endl;
        return;
    }

    m_sheetMap.insert(std::make_pair(sheetID, new SheetData(path, pixelArt)));
}

void AssetManager::AddFont(int fontID, const std::string &assetsPath, const std::string &fileName, float size)
{
    const std::string path = assetsPath + fileName;
    auto it = m_fontMap.find(fontID);
    if (it != m_fontMap.end())
    {
        std::cout
            << "ERROR - Add font " << path << std::endl
            << "      - The font already exists" << std::endl;
        return;
    }

    m_fontMap.insert(std::make_pair(fontID, new FontData(path, size)));
}

void AssetManager::AddSound(int soundID, const std::string &assetsPath, const std::string &fileName)
{
    const std::string path = assetsPath + fileName;
    auto it = m_soundMap.find(soundID);
    if (it != m_soundMap.end())
    {
        std::cout
            << "ERROR - Add sound " << path << std::endl
            << "      - The sound already exists" << std::endl;
        return;
    }

    m_soundMap.insert(std::make_pair(soundID, new SoundData(path)));
}

void AssetManager::AddMusic(int musicID, const std::string &assetsPath, const std::string &fileName)
{    
    const std::string path = assetsPath + fileName;
    auto it = m_musicMap.find(musicID);
    if (it != m_musicMap.end())
    {
        std::cout
            << "ERROR - Add music " << path << std::endl
            << "      - The music already exists" << std::endl;
        return;
    }

    m_musicMap.insert(std::make_pair(musicID, new MusicData(path)));
}

void AssetManager::AddTexture(int textureID, const std::string &assetsPath, const std::string &fileName, bool pixelArt)
{
    const std::string path = assetsPath + fileName;
    auto it = m_textureMap.find(textureID);
    if (it != m_textureMap.end())
    {
        std::cout
            << "ERROR - Add texture " << path << std::endl
            << "      - The texture already exists" << std::endl;
        return;
    }

    m_textureMap.insert(std::make_pair(textureID, new TextureData(path, pixelArt)));
}

int AssetManager::LoadSpriteSheets(const std::vector<int> &sheetIDs)
{
    int exitStatus = EXIT_SUCCESS;
    for (int sheetID : sheetIDs)
    {
        SpriteSheet *spriteSheet = GetSpriteSheet(sheetID);
        if (spriteSheet == nullptr) exitStatus = EXIT_FAILURE;
    }
    return exitStatus;
}

SDL_Texture *AssetManager::GetTexture(int textureID)
{
    auto it = m_textureMap.find(textureID);
    if (it != m_textureMap.end())
    {
        return it->second->GetTexture();
    }
    return nullptr;
}

SpriteSheet *AssetManager::GetSpriteSheet(int sheetID)
{
    auto it = m_sheetMap.find(sheetID);
    if (it != m_sheetMap.end())
    {
        return it->second->GetSpriteSheet();
    }
    return nullptr;
}

TTF_Font *AssetManager::GetFont(int fontID)
{
    auto it = m_fontMap.find(fontID);
    if (it != m_fontMap.end())
    {
        return it->second->GetFont();
    }
    return nullptr;
}

float AssetManager::GetFontInitialSize(int fontID) const
{
    auto it = m_fontMap.find(fontID);
    if (it != m_fontMap.end())
    {
        return it->second->m_size;
    }
    return 24.f;
}

Mix_Chunk *AssetManager::GetSound(int soundID)
{
    auto it = m_soundMap.find(soundID);
    if (it != m_soundMap.end())
    {
        return it->second->GetSound();
    }
    return nullptr;
}

Mix_Music *AssetManager::GetMusic(int musicID)
{
    auto it = m_musicMap.find(musicID);
    if (it != m_musicMap.end())
    {
        return it->second->GetMusic();
    }
    return nullptr;
}

void AssetManager::InitSoundFXChannels(int startID, int endID)
{
    m_sfxChannels.clear();
    for (int i = startID; i <= endID; i++)
    {
        m_sfxChannels.push_back(i);
    }
    m_sfxIndex = 0;
}

void AssetManager::SetSoundVolume(int soundID, float volume)
{
    volume = math::Clamp(volume, 0.f, 1.f);
    Mix_Chunk *chunk = GetSound(soundID);
    if (chunk == nullptr)
    {
        assert(false);
        return;
    }

#ifdef MIX_MAX_VOLUME
    Mix_VolumeChunk(chunk, (int)(volume * MIX_MAX_VOLUME));
#else
    (void)chunk;
#endif
}

void AssetManager::SetMusicVolume(float volume)
{
    volume = math::Clamp(volume, 0.f, 1.f);
#ifdef MIX_MAX_VOLUME
    Mix_VolumeMusic((int)(volume * MIX_MAX_VOLUME));
#else
    if (g_musicTrack) MIX_SetTrackGain(g_musicTrack, volume);
#endif
}

void AssetManager::SetSoundFXVolume(float volume)
{
    volume = math::Clamp(volume, 0.f, 1.f);
#ifdef MIX_MAX_VOLUME
    int v = (int)(volume * MIX_MAX_VOLUME);
    for (int channel : m_sfxChannels)
    {
        Mix_Volume(channel, v);
    }
#else
    for (int channel : m_sfxChannels)
    {
        if (0 <= channel && channel < MIX_CHANNEL_COUNT && g_sfxTracks[channel])
        {
            MIX_SetTrackGain(g_sfxTracks[channel], volume);
        }
    }
#endif
}

void AssetManager::PlaySoundFX(int soundID, int loop)
{
    assert(m_sfxChannels.size() > 0);
    assert(m_sfxIndex < m_sfxChannels.size());

    Mix_Chunk *chunk = GetSound(soundID);
    if (chunk == nullptr)
    {
        assert(false);
        return;
    }

    int channelID = m_sfxChannels[m_sfxIndex];
#ifdef MIX_MAX_VOLUME
    int exitStatus = Mix_PlayChannel(channelID, chunk, loop);
    (void)exitStatus;
#else
    if (0 <= channelID && channelID < MIX_CHANNEL_COUNT && g_sfxTracks[channelID])
    {
        MIX_SetTrackAudio(g_sfxTracks[channelID], chunk);
        SDL_PropertiesID props = SDL_CreateProperties();
        SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, loop);
        MIX_PlayTrack(g_sfxTracks[channelID], props);
        SDL_DestroyProperties(props);
    }
#endif

    m_sfxIndex = ((size_t)m_sfxIndex + 1) % m_sfxChannels.size();
}

void AssetManager::PlaySound(int soundID, int channelID, int loops)
{
    Mix_Chunk *chunk = GetSound(soundID);
    if (chunk == nullptr)
    {
        assert(false);
        return;
    }

#ifdef MIX_MAX_VOLUME
    int exitStatus = Mix_PlayChannel(channelID, chunk, loops);
    (void)exitStatus;
#else
    if (0 <= channelID && channelID < MIX_CHANNEL_COUNT && g_sfxTracks[channelID])
    {
        MIX_SetTrackAudio(g_sfxTracks[channelID], chunk);
        SDL_PropertiesID props = SDL_CreateProperties();
        SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, loops);
        MIX_PlayTrack(g_sfxTracks[channelID], props);
        SDL_DestroyProperties(props);
    }
#endif
}

void AssetManager::PlayMusic(int musicID, int loops)
{
    Mix_Music *music = GetMusic(musicID);
    if (music == nullptr)
    {
        assert(false);
        return;
    }

#ifdef MIX_MAX_VOLUME
    bool success = Mix_PlayMusic(music, loops);
    assert(success);
#else
    if (g_musicTrack)
    {
        MIX_SetTrackAudio(g_musicTrack, music);
        SDL_PropertiesID props = SDL_CreateProperties();
        SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, loops);
        bool success = MIX_PlayTrack(g_musicTrack, props);
        SDL_DestroyProperties(props);
        assert(success);
    }
#endif
}

void AssetManager::FadeInMusic(int musicID, int loops, int ms, double position)
{
    Mix_Music *music = GetMusic(musicID);
    if (music == nullptr)
    {
        assert(false);
        return;
    }

#ifdef MIX_MAX_VOLUME
    bool success = Mix_FadeInMusicPos(music, loops, ms, position);
    assert(success);
#else
    (void)position;
    if (g_musicTrack)
    {
        MIX_SetTrackAudio(g_musicTrack, music);
        SDL_PropertiesID props = SDL_CreateProperties();
        SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, loops);
        SDL_SetNumberProperty(props, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, ms);
        bool success = MIX_PlayTrack(g_musicTrack, props);
        SDL_DestroyProperties(props);
        assert(success);
    }
#endif
}

void AssetManager::FadeOutMusic(int ms)
{
#ifdef MIX_MAX_VOLUME
    Mix_FadeOutMusic(ms);
#else
    if (g_musicTrack) MIX_StopTrack(g_musicTrack, MIX_TrackMSToFrames(g_musicTrack, ms));
#endif
}

void AssetManager::CreateIOStream(const std::string &path, SDL_IOStream **ioStream, void **buffer)
{
    FILE *file = fopen(path.c_str(), "rb");
    if (file == nullptr)
    {
        std::cout << "ERROR - The file " << path << " cannot be opened" << std::endl;
        assert(false);
        abort();
    }

    fseek(file, 0, SEEK_END);
    size_t size = (size_t)ftell(file);
    rewind(file);
    uint8_t *mem = (uint8_t *)calloc(size, sizeof(uint8_t));
    AssertNew(mem);

    *buffer = (void *)mem;

    size_t freadCount = fread(mem, 1, size, file);
    assert(freadCount == size);
    fclose(file); file = nullptr;

    if (size > 2 && mem[0] == (uint8_t)0x0B && mem[1] == (uint8_t)0xF7)
    {
        // Utilisation du magic number 0x0BF7 pour les fichiers obsfusqués
        mem += 2;
        size -= 2;
        RetriveMem((void *)mem, size);
    }

    *ioStream = SDL_IOFromConstMem(mem, (int)size);

    if (*ioStream == nullptr)
    {
        std::cout
            << "ERROR - Load IOStream " << path << std::endl
            << "      - " << SDL_GetError() << std::endl;
        assert(false);
        abort();
    }
}

void AssetManager::DestroyIOStream(SDL_IOStream *ioStream, void *buffer)
{
    if (ioStream) SDL_CloseIO(ioStream);
    free(buffer);
}

void AssetManager::ObfuscateMem(void *memory, size_t size)
{
    Uint8 *buffer = (Uint8 *)memory;
    buffer[0] ^= 0x73;
    buffer[0] = 0xBB * buffer[0] + 0xC9;
    for (size_t i = 1; i < size; i++)
    {
        buffer[i] ^= buffer[i - 1];
        buffer[i] = 0xBB * buffer[i] + 0xC9;
    }
}

void AssetManager::RetriveMem(void *memory, size_t size)
{
    Uint8 *buffer = (Uint8 *)memory;
    for (size_t i = size - 1; i > 0; i--)
    {
        buffer[i] = 0x73 * (buffer[i] + 0x37);
        buffer[i] ^= buffer[i - 1];
    }
    buffer[0] = 0x73 * (buffer[0] + 0x37);
    buffer[0] ^= 0x73;
}

AssetManager::MusicData::MusicData(const std::string &path)
    : m_path(path)
    , m_music(nullptr)
    , m_ioStream(nullptr)
    , m_ioStreamBuffer(nullptr)
{
}

AssetManager::MusicData::~MusicData()
{
#ifdef MIX_MAX_VOLUME
    if (m_music) Mix_FreeMusic(m_music);
#else
    if (m_music) MIX_DestroyAudio(m_music);
#endif
    DestroyIOStream(m_ioStream, m_ioStreamBuffer);
}

Mix_Music *AssetManager::MusicData::GetMusic()
{
    if (m_music != nullptr) return m_music;

    CreateIOStream(m_path, &m_ioStream, &m_ioStreamBuffer);
#ifdef MIX_MAX_VOLUME
    m_music = Mix_LoadMUS_IO(m_ioStream, 0);
#else
    m_music = MIX_LoadAudio_IO(g_mixer, m_ioStream, false, false);
#endif
    if (m_music == NULL)
    {
        std::cout
            << "ERROR - Load music " << m_path << std::endl
            << "      - " << SDL_GetError() << std::endl;
        assert(false);
        abort();
    }

    return m_music;
}

AssetManager::FontData::FontData(const std::string &path, float size)
    : m_path(path)
    , m_font(nullptr)
    , m_size(size)
    , m_ioStream(nullptr)
    , m_ioStreamBuffer(nullptr)
{
    CreateIOStream(m_path, &m_ioStream, &m_ioStreamBuffer);
    m_font = TTF_OpenFontIO(m_ioStream, false, size);
    if (m_font == NULL)
    {
        std::cout
            << "ERROR - Open font " << path << std::endl
            << "      - " << SDL_GetError() << std::endl;
        assert(false);
        abort();
    }
}

AssetManager::FontData::~FontData()
{
    if (m_font) TTF_CloseFont(m_font);
    DestroyIOStream(m_ioStream, m_ioStreamBuffer);
}

TTF_Font *AssetManager::FontData::GetFont()
{
    return m_font;
}

AssetManager::SoundData::SoundData(const std::string &path) :
    m_chunk(nullptr)
{
    void *ioStreamBuffer = nullptr;
    SDL_IOStream *ioStream = nullptr;
    CreateIOStream(path, &ioStream, &ioStreamBuffer);

#ifdef MIX_MAX_VOLUME
    m_chunk = Mix_LoadWAV_IO(ioStream, 0);
#else
    m_chunk = MIX_LoadAudio_IO(g_mixer, ioStream, true, false);
#endif
    if (m_chunk == NULL)
    {
        std::cout
            << "ERROR - Loading audio " << path << std::endl
            << "      - " << SDL_GetError() << std::endl;
        assert(false);
        abort();
    }

    DestroyIOStream(ioStream, ioStreamBuffer);
}

AssetManager::SoundData::~SoundData()
{
#ifdef MIX_MAX_VOLUME
    if (m_chunk) Mix_FreeChunk(m_chunk);
#else
    if (m_chunk) MIX_DestroyAudio(m_chunk);
#endif
}

Mix_Chunk *AssetManager::SoundData::GetSound()
{
    return m_chunk;
}

AssetManager::SheetData::SheetData(const std::string &path, bool pixelArt)
    : m_path(path)
    , m_sheet(nullptr)
    , m_pixelArt(pixelArt)
{
}

AssetManager::SheetData::~SheetData()
{
    if (m_sheet) delete m_sheet;
}

SpriteSheet *AssetManager::SheetData::GetSpriteSheet()
{
    if (m_sheet) return m_sheet;

    m_sheet = new SpriteSheet(g_renderer, m_path);

    if (m_pixelArt)
    {
        SDL_Texture *texture = m_sheet->GetTexture();
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    }
    return m_sheet;
}

AssetManager::TextureData::TextureData(const std::string &path, bool pixelArt)
    : m_path(path)
    , m_texture(nullptr)
    , m_pixelArt(pixelArt)
{
}

AssetManager::TextureData::~TextureData()
{
    if (m_texture) SDL_DestroyTexture(m_texture);
}

SDL_Texture *AssetManager::TextureData::GetTexture()
{
    if (m_texture) return m_texture;

    void *ioStreamBuffer = NULL;
    SDL_IOStream *ioStream = NULL;
    CreateIOStream(m_path, &ioStream, &ioStreamBuffer);

    SDL_Texture *m_texture = IMG_LoadTexture_IO(g_renderer, ioStream, 0);
    if (m_texture == NULL)
    {
        std::cout
            << "ERROR - Load texture " << m_path << std::endl
            << "      - " << SDL_GetError() << std::endl;
        assert(false);
        abort();
    }

    if (m_pixelArt)
    {
        SDL_SetTextureScaleMode(m_texture, SDL_SCALEMODE_NEAREST);
    }

    DestroyIOStream(ioStream, ioStreamBuffer);

    return m_texture;
}
