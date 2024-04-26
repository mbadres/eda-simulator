/*
 * AnalysisEDA.cpp
 *
 *  Created on: Oct 22, 2019
 *      Author: mbadres
 */

#include "AnalysisEDA.h"

#include "Graph/GraphHandler.h"

#include <iostream>
#include <fstream>

void AnalysisEDA::run()
{
    // initialize vector with right size and logic X as value to save the output signal for every clock signal
    std::vector<std::vector<Logic>> output {inputData.size(), {graphHandler->getAllNets().size(), Logic::logicX}};

    // write signals in output vector
    for (size_t line = 0; line < output.size(); ++line)
    {
        // copy last output to current line
        output[line] = output[(line - 1) % output.size()];

        // update input signals
        for (size_t n = 0, count_input = 0; n < graphHandler->getAllNets().size(); ++n)
            if (graphHandler->getAllNets()[n]->getInElement() == nullptr)
                output[line][n] = inputData[line][count_input++];

        // calculate other signals of net
        for (size_t n = 0; n < graphHandler->getAllNets().size(); ++n)
        {
            // skip input signals
            if (graphHandler->getAllNets()[n]->getInElement() == nullptr)
                continue;

            switch (graphHandler->getAllNets()[n]->getInElement()->getElementInfo()->getType())
            {
                // handle inverter cell
                case ElementType::Not:
                {
                    Logic const & signal = output[line][graphHandler->getAllNets()[n]->getInElement()->getInNets()[0]->getId()];

                    if (signal == Logic::logic0)
                        output[line][n] = Logic::logic1;
                    else if (signal == Logic::logic1)
                        output[line][n] = Logic::logic0;
                    else
                        output[line][n] = Logic::logicX;
                    break;
                }

                // handle and cell
                case ElementType::And:
                {
                    Logic const & signal_a = output[line][graphHandler->getAllNets()[n]->getInElement()->getInNets()[0]->getId()];
                    Logic const & signal_b = output[line][graphHandler->getAllNets()[n]->getInElement()->getInNets()[1]->getId()];

                    if (signal_a == Logic::logic0 || signal_b == Logic::logic0)
                        output[line][n] = Logic::logic0;
                    else if (signal_a == Logic::logic1 && signal_b == Logic::logic1)
                        output[line][n] = Logic::logic1;
                    else
                        output[line][n] = Logic::logicX;
                    break;
                }

                // handle or cell
                case ElementType::Or:
                {
                    Logic const & signal_a = output[line][graphHandler->getAllNets()[n]->getInElement()->getInNets()[0]->getId()];
                    Logic const & signal_b = output[line][graphHandler->getAllNets()[n]->getInElement()->getInNets()[1]->getId()];

                    if (signal_a == Logic::logic1 || signal_b == Logic::logic1)
                        output[line][n] = Logic::logic1;
                    else if (signal_a == Logic::logic0 && signal_b == Logic::logic0)
                        output[line][n] = Logic::logic0;
                    else
                        output[line][n] = Logic::logicX;
                    break;
                }

                // handle delay flip flop
                case ElementType::Dff:
                {
                    Logic const & clock = output[line][graphHandler->getAllNets()[n]->getInElement()->getInNets()[0]->getId()];
                    Logic const & signal = output[(line - 1) % output.size()][graphHandler->getAllNets()[n]->getInElement()->getInNets()[1]->getId()];

                    if (clock == Logic::logic1)
                        output[line][n] = signal;
                    else
                        output[line][n] = output[line - 1][n];
                    break;
                }

                //  handle top level cell
                case ElementType::TopLevelCell:
                {
                    // nothing to do, this cell do not exist in given circuits
                    break;
                }

                // unknown gate
                case ElementType::Unknown:
                {
                    // nothing to do, this cell do not exist in given circuits
                    break;
                }
            }
        }
    }

    // std output
    for (std::vector<Logic> const & net : output)
    {
        for (Net const * n : graphHandler->getAllNets())
            if (n->getOutElements()[0] == nullptr)
                std::cout << net[n->getId()] << ",";
        std::cout << std::endl;
    }

    // create output file
    std::ofstream file{"output.csv"};
    for (std::vector<Logic> const & net : output)
    {
        for (Net const * n : graphHandler->getAllNets())
            if (n->getOutElements()[0] == nullptr)
                file << net[n->getId()] << ",";
        file << std::endl;
    }
    file.close();
}
