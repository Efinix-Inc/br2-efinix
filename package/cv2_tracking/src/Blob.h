#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;

class Blob {
    public:
        vector<Point> contour;
        Rect boundingRect;
        Point centerPosition;
        double dblDiagonalSize;
        double dblAspectRatio;

        Blob(vector<Point> _contour);
};
