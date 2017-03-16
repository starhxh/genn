#pragma once

// Standard includes
#include <map>
#include <set>
#include <string>
#include <vector>

// GeNN includes
#include "newNeuronModels.h"

//------------------------------------------------------------------------
// NeuronGroup
//------------------------------------------------------------------------
class NeuronGroup
{
public:
    NeuronGroup(int numNeurons, const NeuronModels::Base *neuronModel,
                const std::vector<double> &params, const std::vector<double> &initVals) :
        m_NumNeurons(numNeurons), m_CumSumNeurons(0), m_PaddedCumSumNeurons(0),
        m_NeuronModel(neuronModel), m_Params(params), m_InitVals(initVals),
        m_NeedSpikeTime(false), m_NeedTrueSpike(false), m_NeedSpikeEvents(false), m_NeedQueue(false),
        m_NumDelaySlots(1),
        m_SpikeZeroCopy(false), m_SpikeEventZeroCopy(false), m_SpikeTimeZeroCopy(false),
        m_HostID(0), m_DeviceID(0)
    {
        m_VarNeedQueue.resize(initVals.size(), false);
    }

    //------------------------------------------------------------------------
    // Public methods
    //------------------------------------------------------------------------
    //!< Checks delay slots currently provided by the neuron group against a required delay and extends if required
    void checkNumDelaySlots(unsigned int requiredDelay);

    // Update which variables require queues based on piece of code
    void updateVarQueues(const std::string &code);

    void setNeedSpikeTiming(){ m_NeedSpikeTime = true; }
    void setNeedTrueSpike(){ m_NeedTrueSpike = true; }
    void setNeedSpikeEvents(){ m_NeedSpikeEvents = true; }

    void setSpikeZeroCopy(){ m_SpikeZeroCopy = true; }
    void setSpikeEventZeroCopy(){ m_SpikeEventZeroCopy = true; }
    void setSpikeTimeZeroCopy(){ m_SpikeTimeZeroCopy = true; }
    void setVarZeroCopy(const std::string &varName);

    void setClusterIndex(int hostID, int deviceID){ m_HostID = hostID; m_DeviceID = deviceID; }

    void addSpkEventCondition(const std::string &code, const std::string &supportCodeNamespace);

    size_t addInSyn(const std::string &synapseName);
    size_t addOutSyn(const std::string &synapseName);

    void initDerivedParams(double dt);
    void calcSizes(unsigned int blockSize, unsigned int &cumSum, unsigned int &paddedCumSum);

    //------------------------------------------------------------------------
    // Public const methods
    //------------------------------------------------------------------------
    unsigned int getNumNeurons() const{ return m_NumNeurons; }
    const NeuronModels::Base *getNeuronModel() const{ return m_NeuronModel; }

    const std::vector<double> &getParams() const{ return m_Params; }
    const std::vector<double> &getDerivedParams() const{ return m_DerivedParams; }
    const std::vector<double> &getInitVals() const{ return m_InitVals; }

    const std::vector<string> &getInSyn() const{ return m_InSyn; }
    const std::vector<string> &getOutSyn() const{ return m_OutSyn; }

    bool doesNeedSpikeTime() const{ return m_NeedSpikeTime; }
    bool doesNeedTrueSpike() const{ return m_NeedTrueSpike; }
    bool doesNeedSpikeEvents() const{ return m_NeedSpikeEvents; }
    bool doesNeedQueue() const{ return m_NeedQueue; }

    bool doesVarNeedQueue(size_t v) const{ return m_VarNeedQueue[v]; }
    bool anyVarNeedQueue() const;

    const std::set<std::pair<std::string, std::string>> &getSpikeEventCondition() const{ return m_SpikeEventCondition; }

    unsigned int getNumDelaySlots() const{ return m_NumDelaySlots; }
    bool delayRequired() const{ return (m_NumDelaySlots > 1); }

    bool usesSpikeZeroCopy() const{ return m_SpikeZeroCopy; }
    bool usesSpikeEventZeroCopy() const{ return m_SpikeEventZeroCopy; }
    bool usesSpikeTimeZeroCopy() const{ return m_SpikeTimeZeroCopy; }
    bool usesZeroCopy() const;

    bool getNumSpikeEventConditions() const{ return m_SpikeEventCondition.size(); }

    void addExtraGlobalParams(const string &groupName, std::map<std::string, std::string> &kernelParameters) const;
    void addSpikeEventConditionParams(const std::pair<std::string, std::string> &param, const std::string &groupName,
                                      std::map<string, string> &kernelParameters) const;


private:
    //------------------------------------------------------------------------
    // Members
    //------------------------------------------------------------------------
    unsigned int m_NumNeurons;
    unsigned int m_CumSumNeurons;
    unsigned int m_PaddedCumSumNeurons;

    const NeuronModels::Base *m_NeuronModel;
    std::vector<double> m_Params;
    std::vector<double> m_DerivedParams;
    std::vector<double> m_InitVals;
    std::vector<std::string> m_InSyn;
    std::vector<std::string> m_OutSyn;
    bool m_NeedSpikeTime;
    bool m_NeedTrueSpike;
    bool m_NeedSpikeEvents;
    bool m_NeedQueue;
    std::set<std::pair<std::string, std::string>> m_SpikeEventCondition;
    unsigned int m_NumDelaySlots;

    //!< Vector specifying which variables require queues
    std::vector<bool> m_VarNeedQueue;

    //!< Whether spikes from neuron group should use zero-copied memory
    bool m_SpikeZeroCopy;

    //!< Whether spike-like events from neuron group should use zero-copied memory
    bool m_SpikeEventZeroCopy;

    //!< Whether spike times from neuron group should use zero-copied memory
    bool m_SpikeTimeZeroCopy;

    //!< Whether indidividual state variables of a neuron group should use zero-copied memory
    std::set<string> m_VarZeroCopy;

    //!< The ID of the cluster node which the neuron groups are computed on
    int m_HostID;

    //!< The ID of the CUDA device which the neuron groups are comnputed on
    int m_DeviceID;
};