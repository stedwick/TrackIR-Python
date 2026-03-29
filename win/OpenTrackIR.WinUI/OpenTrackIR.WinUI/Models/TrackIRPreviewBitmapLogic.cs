namespace OpenTrackIR.WinUI.Models
{
    public static class TrackIRPreviewBitmapLogic
    {
        public static byte[] ExpandGray8ToBgra32(byte[] gray8Pixels)
        {
            ArgumentNullException.ThrowIfNull(gray8Pixels);

            byte[] bgraPixels = new byte[gray8Pixels.Length * 4];
            for (int grayIndex = 0, bgraIndex = 0; grayIndex < gray8Pixels.Length; grayIndex++, bgraIndex += 4)
            {
                byte intensity = gray8Pixels[grayIndex];
                bgraPixels[bgraIndex] = intensity;
                bgraPixels[bgraIndex + 1] = intensity;
                bgraPixels[bgraIndex + 2] = intensity;
                bgraPixels[bgraIndex + 3] = 0xFF;
            }

            return bgraPixels;
        }
    }
}
