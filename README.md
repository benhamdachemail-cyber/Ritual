# RITUAL — custom FL Studio VST3 effect (Windows x64)

**Status:** Complete editable JUCE/C++ source project, **not** a precompiled .vst3. A Windows build and FL Studio smoke test have **not** been performed in this environment. The six controls are implemented as an experimental v1, not emulations of commercial plugins.

## Quick install

1. Install **Visual Studio 2022 Community** with the **Desktop development with C++** workload, **CMake** (3.22 or later, add to PATH), and **Git for Windows** (add to PATH). Links: https://visualstudio.microsoft.com/vs/ , https://cmake.org/download/ , https://git-scm.com/download/win .
2. Extract the ZIP to a short path like `C:\Ritual` (avoid OneDrive and extremely long paths).
3. Double-click **BUILD_AND_INSTALL.bat**. The first build will download JUCE via Git; internet access is required.
4. The batch file copies the generated `Ritual.vst3` bundle to `%LOCALAPPDATA%\Programs\Common\VST3` to avoid requiring administrator permission. **This is not FL Studio's standard VST3 scan location.**
5. Best option: move/copy `Ritual.vst3` (the entire folder) from that location to `C:\Program Files\Common Files\VST3\` (approve Windows UAC). Alternatively, add `%LOCALAPPDATA%\Programs\Common\VST3` as an extra scan folder under FL Studio's **Options > Manage plugins > Plugin search paths**.
6. In **Options > Manage plugins**, click **Find installed plugins**. Load **Ritual** in a Mixer effect slot.

## Controls

- **Reverse:** blends from dry playback to overlapping reverse audio grains. Buffered, so a short onset delay is expected.
- **Half Time:** lowers playback speed of overlapping grains to half speed; may create rhythmic texture/pitch variation rather than perfect time-stretching.
- **Triplet:** adds tempo-synced three-per-beat gated retriggers.
- **Phase Shift:** stereo-moving short comb/phase texture.
- **Flux:** changes the phase modulation speed and stutter feel.
- **Aperture:** bright, gently saturated coloration (0 is bypassed).

Double-click knobs to reset to zero. All six controls automate and save in FL Studio projects through JUCE's parameter/state support.

## Project organization

- `Source/PluginProcessor.*` — real-time DSP and parameter state.
- `Source/PluginEditor.*` — custom pink/salmon metal-style GUI, cyan waveform display and dark hardware knobs.
- `Artwork/theme-reference.png` — original reference image supplied by the user; retained in source package for future edits. GUI is custom drawn, rather than a text-heavy background photo.
- `CMakeLists.txt` — fetches pinned JUCE 8.0.17-security and generates a Visual Studio 2022 VST3 target.
- `BUILD_AND_INSTALL.bat` — Windows build and user-space deployment.

## Troubleshooting

- **cmake/git not found:** reopen terminal after installing them or add them to PATH.
- **Visual Studio generator error:** install Visual Studio 2022 with C++ desktop workload, or adjust `-G` if using a different installed Visual Studio.
- **JUCE download fails:** check Internet, Git, proxy/firewall. Retry.
- **Plugin doesn't show up:** ensure the entire `.vst3` folder is in the default VST3 directory, and re-scan. If necessary run FL Studio's plugin manager scan with errors enabled.
- **Unexpected sound:** this is an experimental buffer/granular effect; use modest amounts, especially combinations of Reverse and Half Time. It isn't guaranteed to preserve pitch/transients.
- **JUCE licensing:** Review JUCE terms before commercial release: https://juce.com/legal/juce-8-licence/ .

No guarantee of first-try compilation is made because a Windows/MSVC test machine is unavailable in this session.

## Build on GitHub without using your PC's storage

1. Create a **public** GitHub repository named `Ritual` (for free GitHub-hosted Actions minutes). Do not add a README when creating it.
2. On your PC, extract this project and upload **the contents inside the `Ritual` folder**, not the containing folder, via the repository's **Add file > Upload files** page. Make sure `CMakeLists.txt` is at the repository root and `Source` is present. Commit to the `main` branch. Uploading with the GitHub website may skip hidden `.github` folders.
3. If `.github/workflows/build-windows.yml` did not upload, in the repository choose **Add file > Create new file**, name it exactly `.github/workflows/build-windows.yml`, paste the contents of that file from your extracted project, and commit to `main`. This commit should start the build automatically. (Alternatively run it using the Actions tab's **Run workflow** button.)
4. Open the repo's **Actions** tab > **Build Ritual Windows VST3** > newest completed run. If it is green, under **Artifacts** download `Ritual-Windows-VST3`.
5. Unzip the artifact. Place the resulting **Ritual.vst3** folder in `C:\Program Files\Common Files\VST3\`. If the ZIP extracts as raw `Contents` or `Resources` folders instead of the `Ritual.vst3` folder, make a folder named `Ritual.vst3` and put them inside it. Then rescan in FL Studio > Options > Manage plugins.

Build requires no JUCE, CMake, or Visual Studio on your PC; GitHub's Windows runner supplies the compiler and builds JUCE in the cloud. A successful Windows build has not been verified yet; a red Actions run means the logs must be inspected and source/build settings corrected. GitHub Actions requires a GitHub account and internet; public repositories make the source code visible to others. GitHub-hosted runner policies and free usage limits can change.
