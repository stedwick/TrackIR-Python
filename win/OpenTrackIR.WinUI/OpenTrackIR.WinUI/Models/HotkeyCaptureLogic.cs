using System.Globalization;

namespace OpenTrackIR.WinUI.Models
{
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
    }
}
