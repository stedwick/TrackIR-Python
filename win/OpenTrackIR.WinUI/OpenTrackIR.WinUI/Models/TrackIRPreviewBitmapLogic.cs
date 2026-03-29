namespace OpenTrackIR.WinUI.Models
{
    public static class TrackIRPreviewBitmapLogic
    {
        public static int Gray8BufferLength(int width, int height)
        {
            return checked(width * height);
        }

        public static int Bgra32BufferLength(int width, int height)
        {
            return checked(Gray8BufferLength(width, height) * 4);
        }

        public static void ExpandGray8ToBgra32(ReadOnlySpan<byte> gray8Pixels, Span<byte> bgraPixels)
        {
            if (bgraPixels.Length < gray8Pixels.Length * 4)
            {
                throw new ArgumentException("Destination buffer is too small.", nameof(bgraPixels));
            }

            for (int grayIndex = 0, bgraIndex = 0; grayIndex < gray8Pixels.Length; grayIndex++, bgraIndex += 4)
            {
                byte intensity = gray8Pixels[grayIndex];
                bgraPixels[bgraIndex] = intensity;
                bgraPixels[bgraIndex + 1] = intensity;
                bgraPixels[bgraIndex + 2] = intensity;
                bgraPixels[bgraIndex + 3] = 0xFF;
            }
        }
    }
}
