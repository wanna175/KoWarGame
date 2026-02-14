using System.IO;
using UnityEngine;

public class KoreaMapData //: MonoBehaviour
{
    public int[,] tiles; // CSV 읽어서 저장
    public int width;
    public int height;

    // CSV 읽기
    public void LoadCSV(string filePath)
    {
        if (!File.Exists(filePath))
        {
            Debug.LogError($"CSV 파일 없음: {filePath}");
            return;
        }

        string[] lines = File.ReadAllLines(filePath);
        height = lines.Length;
        width = lines[0].Split(',').Length;

        tiles = new int[height, width];

        for (int y = 0; y < height; y++)
        {
            string[] values = lines[y].Split(',');
            for (int x = 0; x < width; x++)
            {
                int flippedY = height - 1 - y;
                if (int.TryParse(values[x], out int v))
                    tiles[flippedY, x] = v;
                else
                    tiles[flippedY, x] = 0; // 실패 시 바다 처리
            }
        }

        Debug.Log($"CSV 로드 완료: {width} x {height}");
    }

    // 예시: 위도/경도 -> 타일 좌표
    public (int x, int y) LatLonToTile(float lat, float lon,
                                      float minLat, float maxLat,
                                      float minLon, float maxLon)
    {
        int tileX = Mathf.FloorToInt((lon - minLon) / (maxLon - minLon) * width);
        int tileY = Mathf.FloorToInt((lat - minLat) / (maxLat - minLat) * height);
        tileX = Mathf.Clamp(tileX, 0, width - 1);
        tileY = Mathf.Clamp(tileY, 0, height - 1);
        return (tileX, tileY);
    }

    // 예시: 위도/경도 -> 육지/바다 판별
    public int GetTileValue(float lat, float lon,
                            float minLat, float maxLat,
                            float minLon, float maxLon)
    {
        var (x, y) = LatLonToTile(lat, lon, minLat, maxLat, minLon, maxLon);
        return tiles[y, x]; // 0=바다, 1=육지
    }
}
