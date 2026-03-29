using System.Globalization;

namespace OpenTrackIR.WinUI.Models
{
    public readonly record struct RegisteredHotkey(
        bool IsControlPressed,
        bool IsAltPressed,
        bool IsShiftPressed,
        bool IsWindowsPressed,
        int VirtualKeyCode
    );

    public static class HotkeyCaptureLogic
    {
        public static string? FormatHotkeyText(
            bool isControlPressed,
            bool isAltPressed,
            bool isShiftPressed,
            bool isWindowsPressed,
            string? keyToken
        )
        {
            if (string.IsNullOrWhiteSpace(keyToken))
            {
                return null;
            }

            List<string> parts = new();
            if (isControlPressed)
            {
                parts.Add("Ctrl");
            }

            if (isAltPressed)
            {
                parts.Add("Alt");
            }

            if (isShiftPressed)
            {
                parts.Add("Shift");
            }

            if (isWindowsPressed)
            {
                parts.Add("Win");
            }

            parts.Add(keyToken.Trim());
            return string.Join("+", parts);
        }

        public static bool IsModifierKey(int virtualKeyCode)
        {
            return virtualKeyCode is 16 or 17 or 18 or 91 or 92;
        }

        public static string? KeyTokenForVirtualKey(int virtualKeyCode)
        {
            if (virtualKeyCode is >= 65 and <= 90)
            {
                return ((char)virtualKeyCode).ToString();
            }

            if (virtualKeyCode is >= 48 and <= 57)
            {
                return ((char)virtualKeyCode).ToString();
            }

            if (virtualKeyCode is >= 112 and <= 123)
            {
                return "F" + (virtualKeyCode - 111).ToString(CultureInfo.InvariantCulture);
            }

            return virtualKeyCode switch
            {
                8 => "Backspace",
                9 => "Tab",
                13 => "Enter",
                19 => "Pause",
                20 => "CapsLock",
                27 => "Esc",
                32 => "Space",
                33 => "PageUp",
                34 => "PageDown",
                35 => "End",
                36 => "Home",
                37 => "Left",
                38 => "Up",
                39 => "Right",
                40 => "Down",
                45 => "Insert",
                46 => "Delete",
                106 => "Num*",
                107 => "Num+",
                109 => "Num-",
                110 => "Num.",
                111 => "Num/",
                186 => ";",
                187 => "=",
                188 => ",",
                189 => "-",
                190 => ".",
                191 => "/",
                192 => "`",
                219 => "[",
                220 => "\\",
                221 => "]",
                222 => "'",
                _ => null,
            };
        }

        public static bool TryParseHotkeyText(string? hotkeyText, out RegisteredHotkey hotkey)
        {
            hotkey = default;
            if (string.IsNullOrWhiteSpace(hotkeyText))
            {
                return false;
            }

            bool isControlPressed = false;
            bool isAltPressed = false;
            bool isShiftPressed = false;
            bool isWindowsPressed = false;
            int? virtualKeyCode = null;
            string[] parts = hotkeyText.Split('+', StringSplitOptions.TrimEntries | StringSplitOptions.RemoveEmptyEntries);

            foreach (string part in parts)
            {
                switch (part.ToUpperInvariant())
                {
                    case "CTRL":
                    case "CONTROL":
                        isControlPressed = true;
                        break;
                    case "ALT":
                        isAltPressed = true;
                        break;
                    case "SHIFT":
                        isShiftPressed = true;
                        break;
                    case "WIN":
                    case "WINDOWS":
                        isWindowsPressed = true;
                        break;
                    default:
                        if (virtualKeyCode.HasValue)
                        {
                            return false;
                        }

                        int? candidateVirtualKeyCode = VirtualKeyForKeyToken(part);
                        if (!candidateVirtualKeyCode.HasValue)
                        {
                            return false;
                        }

                        virtualKeyCode = candidateVirtualKeyCode.Value;
                        break;
                }
            }

            if (!virtualKeyCode.HasValue)
            {
                return false;
            }

            hotkey = new RegisteredHotkey(
                IsControlPressed: isControlPressed,
                IsAltPressed: isAltPressed,
                IsShiftPressed: isShiftPressed,
                IsWindowsPressed: isWindowsPressed,
                VirtualKeyCode: virtualKeyCode.Value
            );
            return true;
        }

        public static int? VirtualKeyForKeyToken(string? keyToken)
        {
            if (string.IsNullOrWhiteSpace(keyToken))
            {
                return null;
            }

            string trimmedToken = keyToken.Trim();
            if (trimmedToken.Length == 1)
            {
                char character = trimmedToken[0];
                if (char.IsLetter(character))
                {
                    return char.ToUpperInvariant(character);
                }

                if (char.IsDigit(character))
                {
                    return character;
                }
            }

            if (trimmedToken.Length >= 2 &&
                trimmedToken.StartsWith("F", StringComparison.OrdinalIgnoreCase) &&
                int.TryParse(trimmedToken[1..], NumberStyles.None, CultureInfo.InvariantCulture, out int functionKeyIndex) &&
                functionKeyIndex is >= 1 and <= 12)
            {
                return 111 + functionKeyIndex;
            }

            return trimmedToken.ToUpperInvariant() switch
            {
                "BACKSPACE" => 8,
                "TAB" => 9,
                "ENTER" => 13,
                "PAUSE" => 19,
                "CAPSLOCK" => 20,
                "ESC" => 27,
                "SPACE" => 32,
                "PAGEUP" => 33,
                "PAGEDOWN" => 34,
                "END" => 35,
                "HOME" => 36,
                "LEFT" => 37,
                "UP" => 38,
                "RIGHT" => 39,
                "DOWN" => 40,
                "INSERT" => 45,
                "DELETE" => 46,
                "NUM*" => 106,
                "NUM+" => 107,
                "NUM-" => 109,
                "NUM." => 110,
                "NUM/" => 111,
                ";" => 186,
                "=" => 187,
                "," => 188,
                "-" => 189,
                "." => 190,
                "/" => 191,
                "`" => 192,
                "[" => 219,
                "\\" => 220,
                "]" => 221,
                "'" => 222,
                _ => null,
            };
        }
    }
}
