# Third-party assets and audio dependencies

CavernBloom's current visual presentation is original procedural geometry. The only external media included are the following four unmodified Ogg files, copied from the user's local official Kenney packs and renamed for their gameplay purpose. No pack archives, preview tracks, external artwork, or background music are included.

## Audio assets

Creator for every file: **Kenney (Kenney Vleugels)**. Each pack's included `License.txt` and official source page identify **Creative Commons CC0 1.0 / public domain**. Attribution is **not required**; this record preserves provenance voluntarily. See the [CC0 dedication](https://creativecommons.org/publicdomain/zero/1.0/).

| Project filename | Original filename (path inside pack) | Source pack / official page | License | Attribution required | Purpose |
| --- | --- | --- | --- | --- | --- |
| `assets/audio/jump.ogg` | `Audio/rollover1.ogg` | [UI Audio](https://kenney.nl/assets/ui-audio) | CC0 1.0 / public domain | No | Accepted grounded jump; 0.227 s |
| `assets/audio/collect.ogg` | `Audio/impactBell_heavy_004.ogg` | [Impact Sounds](https://kenney.nl/assets/impact-sounds) | CC0 1.0 / public domain | No | Each newly collected flower; 0.301 s |
| `assets/audio/damage.ogg` | `Audio/impactPunch_medium_000.ogg` | [Impact Sounds](https://kenney.nl/assets/impact-sounds) | CC0 1.0 / public domain | No | Enemy or thorn contact respawn; 0.431 s |
| `assets/audio/win.ogg` | `Audio/Pizzicato jingles/jingles_PIZZI03.ogg` | [Music Jingles](https://kenney.nl/assets/music-jingles) | CC0 1.0 / public domain | No | One-shot Playing-to-Won cue; 1.147 s |

Files remain byte-identical to their sources. Durations are rounded from Ogg stream metadata. All four are stereo, 44.1 kHz Ogg Vorbis. The Music Jingles file is used only as a short completion effect.

## Audio software dependencies

These are software, not media assets. CMake FetchContent retrieves their source into the ignored build tree; no precompiled binaries or vendored dependency artifacts are tracked.

| Project | Pinned version | Official source | Chosen license option | Role |
| --- | --- | --- | --- | --- |
| miniaudio, David Reid | `0.11.23` tag | [mackron/miniaudio](https://github.com/mackron/miniaudio/tree/0.11.23) | **MIT No Attribution (MIT-0)**, alternative 2 of the [upstream license](https://github.com/mackron/miniaudio/blob/0.11.23/LICENSE) | Audio engine, device initialization, decoded sound cache, one-shot playback |
| stb_vorbis, Sean Barrett and contributors | `1.22`, exact copy bundled in miniaudio `0.11.23` | [bundled source and license](https://github.com/mackron/miniaudio/blob/0.11.23/extras/stb_vorbis.c); [official stb repository](https://github.com/nothings/stb) | **Public domain / Unlicense**, alternative B in the source | Ogg Vorbis decoding, compiled privately with miniaudio |

miniaudio offers Public Domain/Unlicense or MIT No Attribution; CavernBloom chooses MIT-0. stb_vorbis offers MIT or Public Domain/Unlicense; CavernBloom chooses the latter. Both chosen options permit use without an attribution requirement and include no-warranty terms. Their full license texts remain in the fetched upstream source at the pinned revision.
