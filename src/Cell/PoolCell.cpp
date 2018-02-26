/*
    (C) Copyright 2014 CEA LIST. All Rights Reserved.
    Contributor(s): Olivier BICHLER (olivier.bichler@cea.fr)

    This software is governed by the CeCILL-C license under French law and
    abiding by the rules of distribution of free software.  You can  use,
    modify and/ or redistribute the software under the terms of the CeCILL-C
    license as circulated by CEA, CNRS and INRIA at the following URL
    "http://www.cecill.info".

    As a counterpart to the access to the source code and  rights to copy,
    modify and redistribute granted by the license, users are provided only
    with a limited warranty  and the software's author,  the holder of the
    economic rights,  and the successive licensors  have only  limited
    liability.

    The fact that you are presently reading this means that you have had
    knowledge of the CeCILL-C license and that you accept its terms.
*/

#include "Cell/PoolCell.hpp"

const char* N2D2::PoolCell::Type = "Pool";

N2D2::PoolCell::PoolCell(const std::string& name,
                         unsigned int poolWidth,
                         unsigned int poolHeight,
                         unsigned int nbOutputs,
                         unsigned int strideX,
                         unsigned int strideY,
                         unsigned int paddingX,
                         unsigned int paddingY,
                         Pooling pooling)
    : Cell(name, nbOutputs),
      mPoolWidth(poolWidth),
      mPoolHeight(poolHeight),
      mStrideX(strideX),
      mStrideY(strideY),
      mPaddingX(paddingX),
      mPaddingY(paddingY),
      mPooling(pooling)
{
    // ctor
}

unsigned long long int N2D2::PoolCell::getNbConnections() const
{
    unsigned long long int nbConnections = 0;

    for (unsigned int output = 0; output < mNbOutputs; ++output) {
        for (unsigned int oy = 0; oy < mOutputsHeight; ++oy) {
            for (unsigned int ox = 0; ox < mOutputsWidth; ++ox) {
                const unsigned int sxMax
                    = std::min(mChannelsWidth - ox * mStrideX, mPoolWidth);
                const unsigned int syMax
                    = std::min(mChannelsHeight - oy * mStrideY, mPoolHeight);

                for (unsigned int channel = 0; channel < getNbChannels();
                     ++channel) {
                    if (isConnection(channel, output))
                        nbConnections += sxMax * syMax;
                }
            }
        }
    }

    return nbConnections;
}

void N2D2::PoolCell::writeMap(const std::string& fileName) const
{
    std::ofstream data(fileName.c_str());

    if (!data.good())
        throw std::runtime_error("Could not save map file.");

    Gnuplot::setDefaultOutput("png", "size 800,600 tiny", "png");

    Gnuplot gnuplot;
    gnuplot.set("key off").unset("colorbox");
    gnuplot.setXlabel("Output maps");
    gnuplot.setYlabel("Input maps");
    gnuplot.set("palette", "defined  (0 'white', 1 'black')");
    gnuplot.set("cbrange", "[0:1]");
    gnuplot.set("yrange", "[] reverse");
    gnuplot.set("grid", "front xtics ytics lc rgb 'grey'");

    gnuplot.saveToFile(fileName);

    std::stringstream ytics;
    ytics << "(";

    std::stringstream plotCmd;

    for (unsigned int channel = 0; channel < getNbChannels(); ++channel) {
        for (unsigned int output = 0; output < mNbOutputs; ++output) {
            data << isConnection(channel, output) << " ";
            plotCmd << isConnection(channel, output) << " ";
        }

        if (mNbOutputs == 1)
            plotCmd << "0 ";

        plotCmd << "\n";

        if (channel > 0)
            ytics << ", ";

        data << "\n";
        ytics << "\"" << channel << "\" " << channel;
        /*
                NodeOut* const parent =
           dynamic_cast<NodeOut*>(mInputs[channel](0)->getParent());

                if (channel > 0)
                    ytics << ", ";

                if (parent != NULL) {
                    data << " # " << parent->getCell().getName() << "\n";
                    ytics << "\"" << channel << " (" <<
           parent->getCell().getName() << ")\" " << channel;
                }
                else {
                    data << "\n";
                    ytics << "\"" << channel << " (env)\" " << channel;
                }
        */
    }

    if (getNbChannels() == 1) {
        for (unsigned int output = 0; output < mNbOutputs; ++output)
            plotCmd << "0 ";

        plotCmd << "\n";
    }

    ytics << ")";
    data.close();

    std::stringstream xtics;
    xtics << "(";

    for (unsigned int output = 0; output < mNbOutputs; ++output) {
        if (output > 0)
            xtics << ", ";

        xtics << "\"" << getName() << "(" << output << ")\" " << output;
    }

    xtics << ")";

    gnuplot.set("xtics rotate by 90", xtics.str());
    gnuplot.set("ytics", ytics.str());

    gnuplot.plot("-", "matrix with image");
    gnuplot << plotCmd.str();
    gnuplot << "e";

    Gnuplot::setDefaultOutput();
}

void N2D2::PoolCell::getStats(Stats& stats) const
{
    stats.nbNodes += getNbOutputs() * getOutputsWidth() * getOutputsHeight();
    stats.nbConnections += getNbConnections();
}

void N2D2::PoolCell::setOutputsSize()
{
    mOutputsWidth = (unsigned int)((mChannelsWidth + 2 * mPaddingX
        - mPoolWidth + mStrideX) / (double)mStrideX);
    mOutputsHeight = (unsigned int)((mChannelsHeight + 2 * mPaddingY
        - mPoolHeight + mStrideY) / (double)mStrideY);
}
