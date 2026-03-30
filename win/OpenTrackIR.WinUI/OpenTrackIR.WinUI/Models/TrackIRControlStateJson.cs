using System.Text.Json;
using System.Text.Json.Serialization;

namespace OpenTrackIR.WinUI.Models
{
    public static class TrackIRControlStateJson
    {
        public static string Serialize(TrackIRControlState controlState)
        {
            return JsonSerializer.Serialize(controlState, TrackIRControlStateJsonContext.Default.TrackIRControlState);
        }

        public static TrackIRControlState? Deserialize(string json)
        {
            return JsonSerializer.Deserialize(json, TrackIRControlStateJsonContext.Default.TrackIRControlState);
        }
    }

    [JsonSerializable(typeof(TrackIRControlState))]
    public partial class TrackIRControlStateJsonContext : JsonSerializerContext
    {
    }
}
