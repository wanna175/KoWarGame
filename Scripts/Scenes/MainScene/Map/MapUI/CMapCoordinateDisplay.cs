using UnityEngine;
using UnityEngine.UI;

public class CMapCoordinateDisplay : MonoBehaviour
{
    public CMapController mapController;
    public Text coordText;

    void Update()
    {
        Vector3 mouseWorld = mapController.cam.ScreenToWorldPoint(Input.mousePosition);
        Vector2 latLon = mapController.GetLatLon(mouseWorld);
        coordText.text = $"Lat: {latLon.x:F4}, Lon: {latLon.y:F4}";
    }
}
