#include "Blob.h"

Blob::Blob(std::vector<cv::Point> _contour)
{
    contour = _contour;

    boundingRect = cv::boundingRect(contour);
    centerPosition.x = (boundingRect.x + boundingRect.x + boundingRect.width) / 2;
    centerPosition.y = (boundingRect.y + boundingRect.y + boundingRect.height) /2;

    dblDiagonalSize = sqrt(pow(boundingRect.width, 2) + pow(boundingRect.height, 2));

    dblAspectRatio = (float)boundingRect.width / (float)boundingRect.height;
}
