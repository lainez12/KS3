# KUB3 Configuration Architecture

The configuration subsystem handles loading, validating, auto-repairing, and saving the machine's state and parameters. It is built around a robust, fail-safe architecture designed to prevent data corruption and crashes during partial updates or power loss.

## Core Concepts

The configuration is split into three main domains, each mapped to a physical `.ini` file and a C++ struct:
1. **Hardware** (`hardware.h / hardware.ini`): Physical properties of the machine (motor pitches, encoders, serial numbers).
2. **Process** (`process.h / process.ini`): Software parameters (speeds, tuning gains, coordinates, limit thresholds).
3. **Admin** (`admin.h / admin.ini`): Top-level security and UI toggles.

### 1. `ConfigLoader` (Strict vs. Lenient Mode)
The `ConfigLoader` transforms the `.ini` files into C++ structs. It operates in two modes:

* **Strict Mode (`strict = true`) [Default]**
  Used by the main software (Configurator, Tester, etc.). If a required key is missing or malformed, it throws a `std::runtime_error` immediately. The machine will refuse to boot with a broken configuration, preventing physical hardware damage.
* **Lenient Mode (`strict = false`)**
  Used *only* by the `ConfigGenerator`. If a key is missing, it does not throw. Instead, it leaves the C++ struct field at its hardcoded default value and logs the missing key path to a `std::vector<std::string>`. 

### 2. `ConfigSaver` (Atomic Writes)
Writing directly to `.ini` files is dangerous; if the computer loses power midway, the file is corrupted and the machine is bricked. 
`ConfigSaver` uses **Atomic Swapping**:
1. Writes the entire configuration to a temporary file (`process.ini.tmp`).
2. Flushes the OS buffers to disk.
3. Uses `std::filesystem::rename` to atomically overwrite the real file. 
If it crashes during step 1 or 2, the original config is perfectly safe.

### 3. `ConfigGenerator` (Auto-Repair Tool)
The `Kub3ConfigGenerator` is a CLI tool run at boot or during software updates. 
It loads the configuration in **Lenient Mode**. If the loader detects missing keys (because a software update added new features), the generator receives the list of missing keys, logs them as `[FILLED]`, and uses `ConfigSaver` to write the patched file back to disk. 

This guarantees backward compatibility with older `.ini` files without writing migration scripts.

---

## How to add a new configuration parameter

To add a new setting (e.g., a new pad distance):

1. **Add to C++ Struct (`process.h`)**
   ```cpp
   double my_new_setting_mm = 5.0; // MUST provide a safe compile-time default!
   ```
2. **Add Key Definitions (`keys/process.h`)**
   ```cpp
   #define CONF_PROCESS_MY_NEW_SETTING "my_new_setting_mm"
   ```
3. **Add to `ConfigLoader.cpp`**
   Use the `loadField` helper. This automatically handles the strict/lenient fallback logic.
   ```cpp
   loadField(settings, CONF_PROCESS_MY_NEW_SETTING, config.pad.my_new_setting_mm, strict, "group/my_new_setting_mm", activeLogs, [](const QVariant &v) { return v.toDouble(); });
   ```
4. **Add to `ConfigSaver.cpp`**
   ```cpp
   settings.setValue(CONF_PROCESS_MY_NEW_SETTING, config.pad.my_new_setting_mm);
   ```

That's it! When the app boots, `ConfigGenerator` will automatically notice `my_new_setting_mm` is missing from the user's `process.ini`, initialize it to `5.0`, and save it back to disk.
