#include "SquarifiedAlgorithm.h"
using namespace std;

float sumArray(const vector<float>& row)
{
        float sum = 0;
        for(int i = 0; i < row.size(); ++i)
        {
                sum += row[i];
        }
        return sum;
}

Container Container::cutArea(float area) const
{
        if(this->width >= this->height)
        {
                float areaWidth = area / this->height;
                float newWidth = this->width - areaWidth;
                return Container(this->xoffset+areaWidth, this->yoffset, newWidth, this->height);
        }
        else
        {
                float areaHeight = area / this->width;
                float newHeight = this->height - areaHeight;
                return Container(this->xoffset, this->yoffset+areaHeight, this->width, newHeight);
        }
}

vector<ViewRect> Container::getCoordinates(vector<float>& row)
{
        vector<ViewRect> coords;
        float subxoffset = this->xoffset;
        float subyoffset = this->yoffset;
        float sum = sumArray(row);

        if(this->width >= this->height)
        {
                float areaWidth = sum / this->height;
                for(int i = 0; i < row.size(); ++i)
                {
                        coords.push_back(ViewRect(subxoffset, subyoffset, subxoffset + areaWidth, subyoffset + row[i] / areaWidth));
                        subyoffset += row[i] / areaWidth;
                }
        }
        else
        {
                float areaHeight = sum / this->width;
                for(int i = 0; i < row.size(); ++i)
                {
                        coords.push_back(ViewRect(subxoffset, subyoffset, subxoffset + row[i] / areaHeight, subyoffset + areaHeight));
                        subxoffset += row[i] / areaHeight;
                }
        }

        return coords;
}

float Container::getShortestEdge()
{
        return min(this->height, this->width);
}

void normalized(vector<float>& dataVec, float area)
{
        float sum = sumArray(dataVec);
        float multiplier = area / sum;
        for(int i = 0; i<dataVec.size(); ++i)
        {
                dataVec[i] = dataVec[i] * multiplier;
        }
}

void getTreeMap(vector<ViewRect>& treemap, vector<float> dataVec,
        float width, float height, float xoffset, float yoffset)
{
        normalized(dataVec, width * height);

        vector<float> curRow;
        vector< vector<ViewRect> > rawTreeMap;
        Container container(xoffset, yoffset, width, height);
        squarify(dataVec, curRow, container, rawTreeMap);
        flattenTreeMap(rawTreeMap, treemap);
}

void flattenTreeMap(const vector< vector<ViewRect> >& rawTreeMap, vector<ViewRect>& treemap)
{
        for(int i = 0; i<rawTreeMap.size(); ++i)
        {
                for(int j = 0; j<rawTreeMap[i].size(); ++j)
                {
                        treemap.push_back(rawTreeMap[i][j]);
                }
        }
}

void squarify(vector<float>& dataVec, vector<float>& curRow,
        Container& container, vector< vector<ViewRect> >& stack)
{
        if(dataVec.empty() == true)
        {
                stack.push_back(container.getCoordinates(curRow));
                return;
        }

        float length = container.getShortestEdge();
        float nextnode = dataVec[0];

        if(isImprovesRatio(curRow, nextnode, length))
        {
                curRow.push_back(nextnode);
                dataVec.erase(dataVec.begin());
                squarify(dataVec, curRow, container, stack);
        }
        else
        {
                Container newCon = container.cutArea(sumArray(curRow));
                stack.push_back(container.getCoordinates(curRow));
                vector<float> newRow;
                squarify(dataVec, newRow, newCon, stack);
        }
}

bool isImprovesRatio(const vector<float>& curRow, float nextnode, float length)
{
        if(curRow.empty() == true)
                return true;

        vector<float> newRow(curRow.begin(), curRow.end());
        newRow.push_back(nextnode);

        float newRatio = calculateRatio(newRow, length);
        float curRatio = calculateRatio(curRow, length);
        return curRatio >= newRatio;
}

float calculateRatio(const vector<float>& curRow, int length)
{
        float maxData = *max_element(curRow.begin(), curRow.end());
        float minData = *min_element(curRow.begin(), curRow.end());
        float sum = sumArray(curRow);
        return max((length*length*maxData)/(sum*sum), (sum*sum)/(length*length*minData));
}


// https://www.codeproject.com/articles/7039/squarified-treemaps-in-xaml-c-using-microsoft-long
