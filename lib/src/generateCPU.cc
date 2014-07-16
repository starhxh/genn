/*--------------------------------------------------------------------------
  Author: Thomas Nowotny
  
  Institute: Center for Computational Neuroscience and Robotics
  University of Sussex
  Falmer, Brighton BN1 9QJ, UK 
  
  email to:  T.Nowotny@sussex.ac.uk
  
  initial version: 2010-02-07
  
  --------------------------------------------------------------------------*/

//--------------------------------------------------------------------------
/*! \file generateCPU.cc 

  \brief Functions for generating code that will run the neuron and synapse simulations on the CPU. Part of the code generation section.

*/
//--------------------------------------------------------------------------

#include <string>
#include "CodeHelper.cc"
//--------------------------------------------------------------------------
/*!
  \brief Function that generates the code of the function the will simulate all neurons on the CPU.

*/
//--------------------------------------------------------------------------

void genNeuronFunction(NNmodel &model, //!< Model description 
		       string &path, //!< output stream for code
		       ostream &mos //!< output stream for messages
    )
{
    string name, s, localID;
    unsigned int nt;
    ofstream os;

    name = path + toString("/") + model.name + toString("_CODE/neuronFnct.cc");
    os.open(name.c_str());
    // write header content
    writeHeader(os);
    os << ENDL;
    // compiler/include control (include once)
    os << "#ifndef _" << model.name << "_neuronFnct_cc" << ENDL;
    os << "#define _" << model.name << "_neuronFnct_cc" << ENDL;
    os << ENDL;

    // write doxygen comment
    os << "//-------------------------------------------------------------------------" << ENDL;
    os << "/*! \\file neuronFnct.cc" << ENDL << ENDL;
    os << "\\brief File generated from GeNN for the model " << model.name;
    os << " containing the the equivalent of neuron kernel function for the CPU-only version." << ENDL;
    os << "*/" << ENDL;
    os << "//-------------------------------------------------------------------------" << ENDL << ENDL;
    // header
    os << "void calcNeuronsCPU(";
    for (int i = 0; i < model.neuronGrpN; i++) {
	if (model.neuronType[i] == POISSONNEURON) {
	    // Note: Poisson neurons only used as input neurons; they do not 
	    // receive any inputs
	    os << "unsigned int *rates" << model.neuronName[i] << ", // poisson ";
	    os << "\"rates\" of grp " << model.neuronName[i] << ENDL;
	    os << "unsigned int offset" << model.neuronName[i] << ", // poisson ";
	    os << "\"rates\" offset of grp " << model.neuronName[i] << ENDL;
	}
	if (model.receivesInputCurrent[i] >= 2) {
	    os << model.ftype << " *inputI" << model.neuronName[i] << ", // input current of grp " << model.neuronName[i] << ENDL;
	}
    }
    os << model.ftype << " t)" << ENDL;
    os << OB(51);
    for (int i = 0; i < model.neuronGrpN; i++) {
	nt = model.neuronType[i];
	os << "glbscnt" << model.neuronName[i] << " = 0;" << ENDL;
	if (model.neuronDelaySlots[i] == 1) {
	    os << "glbSpkEvntCnt" << model.neuronName[i] << " = 0;" << ENDL;
	}
	else {
	    os << "spkEvntQuePtr" << model.neuronName[i] << " = (spkEvntQuePtr" << model.neuronName[i] << " + 1) % ";
	    os << model.neuronDelaySlots[i] << ";" << ENDL;
	    os << "glbSpkEvntCnt" << model.neuronName[i] << "[spkEvntQuePtr" << model.neuronName[i] << "] = 0;" << ENDL;
	}
	os << "for (int n = 0; n < " <<  model.neuronN[i] << "; n++)" << OB(10);
	for (int k = 0; k < nModels[nt].varNames.size(); k++) {
	    os << nModels[nt].varTypes[k] << " l" << nModels[nt].varNames[k] << " = ";
	    os << nModels[nt].varNames[k] << model.neuronName[i] << "[";
	    if ((nModels[nt].varNames[k] == "V") && (model.neuronDelaySlots[i] != 1)) {
		os << "(((spkEvntQuePtr" << model.neuronName[i] << " + " << (model.neuronDelaySlots[i] - 1) << ") % ";
		os << model.neuronDelaySlots[i] << ") * " << model.neuronN[i] << ") + ";
	    }
	    os << "n];" << ENDL;
	}
	os << model.ftype << " Isyn = 0;" << ENDL;
    
	if (model.inSyn[i].size() > 0) {
	    //os << "    Isyn = ";
	    for (int j = 0; j < model.inSyn[i].size(); j++) {
		for (int k = 0, l = postSynModels[model.postSynapseType[model.inSyn[i][j]]].varNames.size(); k < l; k++) {
		    os << postSynModels[model.postSynapseType[model.inSyn[i][j]]].varTypes[k] << " lps" << postSynModels[model.postSynapseType[model.inSyn[i][j]]].varNames[k] << j;
		    os << " =" <<  postSynModels[model.postSynapseType[model.inSyn[i][j]]].varNames[k] << model.synapseName[model.inSyn[i][j]] << "[n];" << ENDL;

		}
		os << "Isyn+= ";
      		string psCode = postSynModels[model.postSynapseType[model.inSyn[i][j]]].postSyntoCurrent;

		substitute(psCode, tS("$(inSyn)"), tS("inSyn") + model.neuronName[i]+ tS(j) +tS("[n]"));
	
		for (int k = 0, l = nModels[nt].varNames.size(); k < l; k++) {
		    substitute(psCode, tS("$(") + nModels[nt].varNames[k] + tS(")"),
			       tS("l") + nModels[nt].varNames[k]);
		}
	
		for (int k = 0, l = nModels[nt].pNames.size(); k < l; k++) {
		    substitute(psCode, tS("$(") + nModels[nt].pNames[k] + tS(")"),
			       tS("l") + nModels[nt].pNames[k]);
		}

		for (int k = 0, l = postSynModels[model.postSynapseType[model.inSyn[i][j]]].pNames.size(); k < l; k++) {
		    substitute(psCode, tS("$(") + postSynModels[model.postSynapseType[model.inSyn[i][j]]].pNames[k] + tS(")"), 
			       tS(model.postSynapsePara[model.inSyn[i][j]][k]));
		}  
 
		for (int k = 0, l = postSynModels[model.postSynapseType[model.inSyn[i][j]]].varNames.size(); k < l; k++) {
		    substitute(psCode, tS("$(") + postSynModels[model.postSynapseType[model.inSyn[i][j]]].varNames[k] + tS(")"), 
			       tS("lps")+tS(postSynModels[model.postSynapseType[model.inSyn[i][j]]].varNames[k])+tS(j));
		}  
	
		for (int k = 0; k < postSynModels[model.postSynapseType[model.inSyn[i][j]]].dpNames.size(); ++k)
		    substitute(psCode, tS("$(") + postSynModels[model.postSynapseType[model.inSyn[i][j]]].dpNames[k] + tS(")"), tS(model.dpsp[model.inSyn[i][j]][k]));
		
		os << psCode;

		/*os << "inSyn" << model.neuronName[i] << j << "[n] * (";
		  os << SAVEP(model.synapsePara[model.inSyn[i][j]][0]) << " - lV)";*/


/*substitute(code, tS("lrate"), 
  tS("rates") + model.neuronName[i] + tS("[n + offset") + model.neuronName[i] + tS("]"));
*/

		/*if (j < model.inSyn[i].size() - 1) {
		  os << " + ";
		  }
		  else {*/
		os << ";" << ENDL;
		//}
	    }
	}
	if (model.receivesInputCurrent[i] == 1) { //receives constant input
	    if (model.synapseGrpN == 0) {
		os << "Isyn= " << model.globalInp[i] << ";" << ENDL;
	    }
	    else {
		os << "Isyn+= " << model.globalInp[i] << ";" << ENDL;
	    }
	}    	
	if (model.receivesInputCurrent[i] >= 2) {
	    if (model.synapseGrpN == 0) {
		os << "Isyn = (" << model.ftype << ") inputI" << model.neuronName[i] << "[n];" << ENDL;
	    }
	    else {
		os << "Isyn += (" << model.ftype << ") inputI" << model.neuronName[i] << "[n];" << ENDL;
	    }
	}
	string thcode= nModels[nt].thresholdConditionCode;
	if (nModels[nt].thresholdConditionCode  == tS("")) { //no condition provided
	     cerr << "Warning: No thresholdConditionCode for neuron type :  " << model.neuronType[i]  << " used for " << model.name[i] << " was provided. There will be no spikes detected in this population!" << ENDL;
	}
	else {
	    for (int k = 0, l = nModels[nt].varNames.size(); k < l; k++) {
		substitute(thcode, tS("$(") + nModels[nt].varNames[k] + tS(")"), tS("l")+ nModels[nt].varNames[k]);
	    }
  
	    substitute(thcode, tS("$(Isyn)"), tS("Isyn"));
	    for (int k = 0, l = nModels[nt].pNames.size(); k < l; k++) {
		substitute(thcode, tS("$(") + nModels[nt].pNames[k] + tS(")"),
			   tS(model.neuronPara[i][k]));
	    }
	    
	    for (int k = 0, l = nModels[nt].dpNames.size(); k < l; k++) {
		substitute(thcode, tS("$(") + nModels[nt].dpNames[k] + tS(")"),
			   tS(model.dnp[i][k]));
	    }
	    os << "bool oldSpike= (" << thcode << ");" << ENDL;  
	}
	os << "// calculate membrane potential" << ENDL;
	string code = nModels[nt].simCode;
	for (int k = 0, l = nModels[nt].varNames.size(); k < l; k++) {
	    substitute(code, tS("$(") + nModels[nt].varNames[k] + tS(")"),
		       tS("l") + nModels[nt].varNames[k]);
	}
	if (nt == POISSONNEURON) {
	    substitute(code, tS("lrate"), 
		       tS("rates") + model.neuronName[i] + tS("[n + offset") + model.neuronName[i] + tS("]"));
	}
	substitute(code, tS("$(Isyn)"), tS("Isyn"));
	for (int k = 0, l = nModels[nt].pNames.size(); k < l; k++) {
	    substitute(code, tS("$(") + nModels[nt].pNames[k] + tS(")"), 
		       tS(model.neuronPara[i][k]));
	}
	for (int k = 0, l = nModels[nt].dpNames.size(); k < l; k++) {
	    substitute(code, tS("$(") + nModels[nt].dpNames[k] + tS(")"), 
		       tS(model.dnp[i][k]));
	}
	for (int k = 0, l = nModels[nt].extraGlobalNeuronKernelParameters.size(); k < l; k++) {
	    substitute(code, tS("$(") + nModels[nt].extraGlobalNeuronKernelParameters[k] + tS(")"), 
		       nModels[nt].extraGlobalNeuronKernelParameters[k]+model.neuronName[i]);
	}
	os << code << ENDL;

	// look for spike type events first.
	os << "if (lV >= " << model.nSpkEvntThreshold[i] << ")"<< OB(20);
	os << "// register a spike type event " << ENDL;
	os << "glbSpkEvnt" << model.neuronName[i] << "[";
	if (model.neuronDelaySlots[i] != 1) {
	    os << "(spkEvntQuePtr" << model.neuronName[i] << " * " << model.neuronN[i] << ") + ";
	}
	os << "glbSpkEvntCnt" << model.neuronName[i];
	if (model.neuronDelaySlots[i] != 1) {
	    os << "[spkEvntQuePtr" << model.neuronName[i] << "]";
	}
	os << "++] = n;" << ENDL;
	os << CB(20) << ENDL;

	if (thcode != tS("")) {
	    os << "if ((" << thcode << ") && !(oldSpike)) " << OB(30) << ENDL;
	    os << "// register a true spike" << ENDL;
	    os << "glbSpk" << model.neuronName[i] << "[";
	    os << "glbscnt" << model.neuronName[i] << "++] = n;" << ENDL;
	    if (model.neuronNeedSt[i]) {
		os << "sT" << model.neuronName[i] << "[n] = t;" << ENDL;
	    }
	    if (nModels[nt].resetCode != tS("")) {
		code = nModels[nt].resetCode;
		for (int k = 0, l = nModels[nt].varNames.size(); k < l; k++) {
		    substitute(code, tS("$(") + nModels[nt].varNames[k] + tS(")"), 
			       tS("l")+ nModels[nt].varNames[k]);
		}
		substitute(code, tS("$(Isyn)"), tS("Isyn"));
		for (int k = 0, l = nModels[nt].pNames.size(); k < l; k++) {
		    substitute(code, tS("$(") + nModels[nt].pNames[k] + tS(")"), 
			       tS(model.neuronPara[i][k]));
		}
		for (int k = 0, l = nModels[nt].dpNames.size(); k < l; k++) {
		    substitute(code, tS("$(") + nModels[nt].dpNames[k] + tS(")"), 
			       tS(model.dnp[i][k]));
		}
		os << "        // spike reset code" << ENDL;
		os << "        " << code << ENDL;
	    }
	    os << CB(30) << ENDL;
	}
	for (int k = 0, l = nModels[nt].varNames.size(); k < l; k++) {
	    os << nModels[nt].varNames[k] <<  model.neuronName[i] << "[";
	    if ((nModels[nt].varNames[k] == "V") && (model.neuronDelaySlots[i] != 1)) {
		os << "(spkEvntQuePtr" << model.neuronName[i] << " * " << model.neuronN[i] << ") + ";
	    };
	    os << "n] = l" << nModels[nt].varNames[k] << ";" << ENDL;
	}
	for (int j = 0; j < model.inSyn[i].size(); j++) {
	    string psCode = postSynModels[model.postSynapseType[model.inSyn[i][j]]].postSynDecay;

	    substitute(psCode, tS("$(inSyn)"), tS("inSyn") + model.neuronName[i]+ tS(j) +tS("[n]"));
			
	    for (int k = 0, l = postSynModels[model.postSynapseType[model.inSyn[i][j]]].pNames.size(); k < l; k++) {
		substitute(psCode, tS("$(") + postSynModels[model.postSynapseType[model.inSyn[i][j]]].pNames[k] + tS(")"), 
			   tS(model.postSynapsePara[model.inSyn[i][j]][k]));
	    }  
		   
	    for (int k = 0, l = postSynModels[model.postSynapseType[model.inSyn[i][j]]].varNames.size(); k < l; k++) {
		substitute(psCode, tS("$(") + postSynModels[model.postSynapseType[model.inSyn[i][j]]].varNames[k] + tS(")"), 
			   tS("lps")+tS(postSynModels[model.postSynapseType[model.inSyn[i][j]]].varNames[k])+tS(j));
	    } 
  		 
	    for (int k = 0; k < postSynModels[model.postSynapseType[model.inSyn[i][j]]].dpNames.size(); ++k){
		substitute(psCode, tS("$(") + postSynModels[model.postSynapseType[model.inSyn[i][j]]].dpNames[k] + tS(")"), tS(model.dpsp		[model.inSyn[i][j]][k]));
	    }
	
	    for (int k = 0, l = nModels[nt].varNames.size(); k < l; k++) {
    		substitute(psCode, tS("$(") + nModels[nt].varNames[k] + tS(")"),
			   tS("l") + nModels[nt].varNames[k]);
	    }
	
	    for (int k = 0, l = nModels[nt].pNames.size(); k < l; k++) {
    		substitute(psCode, tS("$(") + nModels[nt].pNames[k] + tS(")"),
			   tS("l") + nModels[nt].pNames[k]);
	    }
    
	    os << psCode;
			
	    /*os << "    inSyn"  << model.neuronName[i] << j << "[n] *= ";
	      unsigned int synID = model.inSyn[i][j];
	      os << SAVEP(model.dsp[synID][0]) << ";" << ENDL;*/
      

	    for (int k = 0, l = postSynModels[model.postSynapseType[model.inSyn[i][j]]].varNames.size(); k < l; k++) {
		os << postSynModels[model.postSynapseType[model.inSyn[i][j]]].varNames[k] << model.synapseName[model.inSyn[i][j]] << "[n]" << " = lps" << postSynModels[model.postSynapseType[model.inSyn[i][j]]].varNames[k] << j << ";" << ENDL;
	    }
	}
	os << CB(10) << ENDL;
	os << ENDL;
    }
    os << CB(51) << ENDL << ENDL;
    os << "#endif" << ENDL;
    os.close();
} 


//--------------------------------------------------------------------------
/*!
  \brief Function that generates code that will simulate all synapses of the model on the CPU.

*/
//--------------------------------------------------------------------------

void genSynapseFunction(NNmodel &model, //!< Model description
			string &path, //!< Path for code generation
			ostream &mos //!< output stream for messages
    )
{
    string name, s, localID, theLG;
    ofstream os;

    name = path + toString("/") + model.name + toString("_CODE/synapseFnct.cc");
    os.open(name.c_str());
    // write header content
    writeHeader(os);
 
    // compiler/include control (include once)
    os << "#ifndef _" << model.name << "_synapseFnct_cc" << ENDL;
    os << "#define _" << model.name << "_synapseFnct_cc" << ENDL;
    os << ENDL;

    // write doxygen comment
    os << "//-------------------------------------------------------------------------" << ENDL;
    os << "/*! \\file synapseFnct.cc" << ENDL << ENDL;
    os << "\\brief File generated from GeNN for the model " << model.name << " containing the equivalent of the synapse kernel and learning kernel functions for the CPU only version." << ENDL;
    os << "*/" << ENDL;
    os << "//-------------------------------------------------------------------------" << ENDL;

    // Function for calculating synapse input to neurons
    // Function header
    os << "void calcSynapsesCPU(" << model.ftype << " t)" << ENDL;
    os << OB(1001);
    if (model.lrnGroups > 0) {
	os << model.ftype << " dt, dg;" << ENDL;
    }
    os << "int ipost, npost";
    if (model.needSynapseDelay) {
	os << ", delaySlot";
    }
    os << ";" << ENDL;
    os << ENDL;
    for (int i = 0; i < model.neuronGrpN; i++) {
	if (model.inSyn[i].size() > 0) { // there is input onto this neuron group
	    for (int j = 0; j < model.inSyn[i].size(); j++) {
		unsigned int synID = model.inSyn[i][j];
		unsigned int src = model.synapseSource[synID];
		float Epre = model.synapsePara[synID][1];
		float Vslope;
		if (model.synapseType[synID] == NGRADSYNAPSE) {
		    Vslope = model.synapsePara[synID][3];
		}
		if (model.neuronDelaySlots[src] != 1) {
		    os << "delaySlot = (spkEvntQuePtr" << model.neuronName[src] << " + ";
		    os << (int) (model.neuronDelaySlots[src] - model.synapseDelay[synID] + 1);
		    os << ") % " << model.neuronDelaySlots[src] << ";" << ENDL;
		}	
		os << "for (int j = 0; j < glbSpkEvntCnt" << model.neuronName[src];
		if (model.neuronDelaySlots[src] != 1) {
		    os << "[delaySlot]";
		}
		os << "; j++) " << OB(201);
		if (model.synapseConnType[synID] == SPARSE) {
		    os << "npost = g" << model.synapseName[synID] << ".gIndInG[glbSpkEvnt" << model.neuronName[src] << "[";
		    if (model.neuronDelaySlots[src] != 1) {
			os << "delaySlot * " << model.neuronN[src] << " + ";
		    }
		    os << "j] + 1] - g" << model.synapseName[synID] << ".gIndInG[glbSpkEvnt" << model.neuronName[src] << "[";
		    if (model.neuronDelaySlots[src] != 1) {
			os << "delaySlot * " << model.neuronN[src] << " + ";
		    }
		    os << "j]];" << ENDL;
		    os << "for (int l = 0; l < npost; l++)" << OB(202);
		    os << "ipost = g" << model.synapseName[synID] << ".gInd[g" << model.synapseName[synID];
		    os << ".gIndInG[glbSpkEvnt" << model.neuronName[src] << "[";
		    if (model.neuronDelaySlots[src] != 1) {
			os << "delaySlot * " << model.neuronN[src] << " + ";
		    }
		    os << "j]] + l];" << ENDL;
		    //os << "      cerr << \" src queue dest glbSpkEvntCnt npost: \" << j << \" \" << l << \" \" << ipost;
		    //os << \" \" << glbSpkEvntCntPN << \" \" << npost <<  ENDL;" << ENDL;
		}
		else {
		    os << "for (int n = 0; n < " << model.neuronN[i] << "; n++)" << OB(202);
		}
		if (model.synapseGType[synID] == INDIVIDUALID) {
		    os << "unsigned int gid = (glbSpkEvnt" << model.neuronName[src] << "[";
		    if (model.neuronDelaySlots[src] != 1) {
			os << "delaySlot * " << model.neuronN[src] << " + ";
		    }
		    os << "j] * " << model.neuronN[i] << " + n);" << ENDL;
		}
		if (model.neuronType[src] != POISSONNEURON) {
		    os << "if ";
		    if (model.synapseGType[synID] == INDIVIDUALID) {
			os << "((B(gp" << model.synapseName[synID] << "[gid >> " << logUIntSz << "], gid & "; 
			os << UIntSz - 1 << ")) && ";
		    }
		    os << "(V" << model.neuronName[src] << "[";
		    if (model.neuronDelaySlots[src] != 1) {
			os << "(delaySlot * " << model.neuronN[src] << ") + ";
		    }
		    os << "glbSpkEvnt" << model.neuronName[src] << "[";
		    if (model.neuronDelaySlots[src] != 1) {
			os << "(delaySlot * " << model.neuronN[src] << ") + ";
		    }
		    os << "j]] > " << Epre << ")";
		    if (model.synapseGType[synID] == INDIVIDUALID) {
			os << ")";
		    }
		    os << OB(204);
		}
		else {
		    if (model.synapseGType[synID] == INDIVIDUALID) {
			os << "if (B(gp" << model.synapseName[synID] << "[gid >> " << logUIntSz << "], gid & ";
			os << UIntSz - 1 << "))" << OB(204);
		    }
		}
		if (model.synapseConnType[synID] != SPARSE) {
		    os << "ipost = n;" << ENDL;
		}
		if (model.synapseGType[synID] == INDIVIDUALG) {
		    if (model.synapseConnType[synID] == SPARSE) {
			theLG = toString("g") + model.synapseName[synID] + toString(".gp[");
			theLG += toString("g") + model.synapseName[synID] + toString(".gIndInG[glbSpkEvnt");
			theLG += toString(model.neuronName[src]) + toString("[");
			if (model.neuronDelaySlots[src] != 1) {
			    theLG += toString("delaySlot * ") + toString(model.neuronN[src]) + toString(" + ");
			}
			theLG += toString("j]] + l]");
		    }
		    else {
			theLG = toString("gp") + model.synapseName[synID] + toString("[glbSpkEvnt") + model.neuronName[src] + toString("[");
			if (model.neuronDelaySlots[src] != 1) {
			    theLG += toString("delaySlot * ") + toString(model.neuronN[src]) + toString(" + ");
			}
			theLG += toString("j] * ") + toString(model.neuronN[i]) + toString(" + ipost]");
		    }
		}
		if ((model.synapseGType[synID] == GLOBALG) || (model.synapseGType[synID] == INDIVIDUALID)) {
		    theLG = toString(model.g0[synID]);
		}
		if ((model.synapseType[synID] == NSYNAPSE) || (model.synapseType[synID] == LEARN1SYNAPSE)) {
		    os << "inSyn" << model.neuronName[i] << j << "[ipost] += " << theLG << ";" << ENDL;
		}
		if (model.synapseType[synID] == NGRADSYNAPSE) {
		    os << "inSyn" << model.neuronName[i] << j << "[ipost] += " << theLG << " * tanh((";
		    if (model.neuronType[src] == POISSONNEURON) {
			os << SAVEP(model.neuronPara[src][2]) << " - " << SAVEP(Epre);
		    }
		    else {
			os << "V" << model.neuronName[src] << "[";
			if (model.neuronDelaySlots[src] != 1) {
			    os << "(delaySlot * " << model.neuronN[src] << ") + ";
			}
			os << "glbSpkEvnt" << model.neuronName[src] << "[";
			if (model.neuronDelaySlots[src] != 1) {
			    os << "(delaySlot * " << model.neuronN[src] << ") + ";
			}
			os << "j]] - " << SAVEP(Epre);
		    }
		    os << ") / " << SAVEP(Vslope) << ");" << ENDL;
		}

		if ((model.neuronType[src] != POISSONNEURON) ||
		    (model.synapseGType[synID] == INDIVIDUALID)) {
		    os << CB(204) << ENDL;
		}
	
	
		os << CB(202);
		os << CB(201);

		//learning using real spikes
		// if needed, do some learning (this is for pre-synaptic spikes)
		if (model.synapseType[synID] == LEARN1SYNAPSE) {
		    os << "for (int j = 0; j < glbscnt" << model.neuronName[src];
		    if (model.neuronDelaySlots[src] != 1) {
			os << "[delaySlot]";
		    }
		    os << "; j++) " << OB(2011);
	
		    if (model.synapseConnType[synID] == SPARSE) {
			os << "npost = g" << model.synapseName[synID] << ".gIndInG[glbSpk" << model.neuronName[src] << "[";
			if (model.neuronDelaySlots[src] != 1) {
			    os << "delaySlot * " << model.neuronN[src] << " + ";
			}
			os << "j] + 1] - g" << model.synapseName[synID] << ".gIndInG[glbSpk" << model.neuronName[src] << "[";
			if (model.neuronDelaySlots[src] != 1) {
			    os << "delaySlot * " << model.neuronN[src] << " + ";
			}
			os << "j]];" << ENDL;
			os << "for (int l = 0; l < npost; l++)" << OB(2021);
			os << "ipost = g" << model.synapseName[synID] << ".gInd[g" << model.synapseName[synID];
			os << ".gIndInG[glbSpk" << model.neuronName[src] << "[";
			if (model.neuronDelaySlots[src] != 1) {
			    os << "delaySlot * " << model.neuronN[src] << " + ";
			}
			os << "j]] + l];" << ENDL;
		    }
		    else {
			os << "for (int n = 0; n < " << model.neuronN[i] << "; n++)" << OB(2021);
		    }
		    if (model.synapseConnType[synID] != SPARSE) {
			os << "ipost = n;" << ENDL;
		    }    

		    // simply assume INDIVIDUALG for now
		    os << "dt = sT" << model.neuronName[i] << "[ipost] - t - ";
		    os << SAVEP(model.synapsePara[synID][11]) << ";" << ENDL;
		    os << "if (dt > " << model.dsp[synID][1] << ")" << OB(71);
		    os << "dg = -" << SAVEP(model.dsp[synID][5]) << ";" << ENDL;
		    os << CB(71);
		    os << "else if (dt > 0.0)"<< OB(72);
		    os << "dg = " << SAVEP(model.dsp[synID][3]) << " * dt + ";
		    os << SAVEP(model.dsp[synID][6]) << ";" << ENDL;
		    os << CB(72);
		    os << "else if (dt > " << model.dsp[synID][2] << ")"<< OB(73);
		    os << "dg = " << SAVEP(model.dsp[synID][4]) << " * dt + ";
		    os << SAVEP(model.dsp[synID][6]) << ";" << ENDL;
		    os << CB(73);
		    os << "else " << OB(74);
		    os << "dg = -" << SAVEP(model.dsp[synID][7]) << ";" << ENDL;
		    os << CB(74);
		    os << "grawp" << model.synapseName[synID] << "[glbSpk" << model.neuronName[src] << "[";
		    if (model.neuronDelaySlots[src] != 1) {
			os << "delaySlot * " << model.neuronN[src] << " + ";
		    }
		    os << "j] * " << model.neuronN[i] << " + ipost] += dg;" << ENDL;
		    if (model.synapseConnType[synID] == SPARSE) {
			os << "g" << model.synapseName[synID] << ".gp[glbSpk" << model.neuronName[src] << "[";
		    }
		    else {
			os << "gp" << model.synapseName[synID] << "[glbSpk" << model.neuronName[src] << "[";
		    }
		    if (model.neuronDelaySlots[src] != 1) {
			os << "delaySlot * " << model.neuronN[src] << " + ";
		    }
		    os << "j] * " << model.neuronN[i] << " + n] += dg;" << ENDL;
		    os << "gp" << model.synapseName[synID] << "[glbSpk" << model.neuronName[src] << "[";
		    if (model.neuronDelaySlots[src] != 1) {
			os << "delaySlot * " << model.neuronN[src] << " + ";
		    }
		    os << "j] * " << model.neuronN[i] << " + n] = ";
		    os << "gFunc" << model.synapseName[synID] << "(grawp" << model.synapseName[synID];
		    os << "[glbSpk" << model.neuronName[src] << "[";
		    if (model.neuronDelaySlots[src] != 1) {
			os << "delaySlot * " << model.neuronN[src] << " + ";
		    }
		    os << "j] * " << model.neuronN[i] << " + n]);" << ENDL; 

		    os << CB(2021);   
		    os << CB(2011);
		}
	    }
	}
    }  
    os << CB(1001);

    if (model.lrnGroups > 0) {
	// function for learning synapses, post-synaptic spikes
	// function header
	os << "void learnSynapsesPostHost(" << model.ftype << " t)" << ENDL;
	os << OB(811);
	os << model.ftype << " dt, dg;" << ENDL;
	os << ENDL;

	for (int i = 0; i < model.lrnGroups; i++) {
	    unsigned int k = model.lrnSynGrp[i];
	    unsigned int src = model.synapseSource[k];
	    unsigned int nN = model.neuronN[src];
	    unsigned int trg = model.synapseTarget[k];
	    float Epre = model.synapsePara[k][1];

	    os << "for (int j = 0; j < glbscnt" << model.neuronName[trg];
	    if (model.neuronDelaySlots[trg] != 1) {
		os << "[spkQuePtr" << model.neuronName[trg] << "]" << ENDL;
	    }
	    os << "; j++)" << OB(910);
	    os << "for (int n = 0; n < " << model.neuronN[src] << "; n++)" << OB(121);
	    os << "if (V" << model.neuronName[trg] << "[";
	    if (model.neuronDelaySlots[trg] != 1) {
		os << "(spkQuePtr" << model.neuronName[trg] << " * " << model.neuronN[trg] << ") + ";
	    }
	    os << "glbSpk" << model.neuronName[trg] << "[";
	    if (model.neuronDelaySlots[trg] != 1) {
		os << "(spkQuePtr" << model.neuronName[trg] << " * " << model.neuronN[trg] << ") + ";
	    }
	    os << "j]] > " << Epre << ")" << OB(131);
	    os << "dt = t - sT" << model.neuronName[src] << "[n]";
	    if (model.neuronDelaySlots[src] != 1) {
		os << " + " << (DT * model.synapseDelay[k]);
	    }
	    os << " - " << SAVEP(model.synapsePara[k][11]) << ";" << ENDL;
	    os << "if (dt > " << model.dsp[k][1] << ")"<< OB(151);
	    os << "dg = -" << SAVEP(model.dsp[k][5]) << ";" << ENDL;
	    os << CB(151);
	    os << "else if (dt > 0.0)"<< OB(161);
	    os << "dg = " << SAVEP(model.dsp[k][3]) << " * dt + " << SAVEP(model.dsp[k][6]) << ";" << ENDL;
	    os << CB(161);
	    os << "else if (dt > " << model.dsp[k][2] << ")"<< OB(171);
	    os << "dg = " << SAVEP(model.dsp[k][4]) << " * dt + " << SAVEP(model.dsp[k][6]) << ";" << ENDL;
	    os << CB(171);
	    os << "else" << OB(181);
	    os << "dg = -" << SAVEP(model.dsp[k][7]) << ";" << ENDL;
	    os << CB(181);
	    os << "grawp" << model.synapseName[k] << "[n * ";
	    os << model.neuronN[trg] << " + glbSpk" << model.neuronName[trg] << "[";
	    if (model.neuronDelaySlots[trg] != 1) {
		os << "(spkEvntQuePtr" << model.neuronName[trg] << " * " << model.neuronN[trg] << ") + ";
	    }
	    os << "j]] += dg;" << ENDL;
	    if (model.synapseConnType[k] == SPARSE) {
		os << "g" << model.synapseName[k] << ".gp[n * ";
	    }
	    else {
		os << "gp" << model.synapseName[k] << "[n * ";
	    }
	    os << model.neuronN[trg] << " + glbSpk" << model.neuronName[trg] << "[";
	    if (model.neuronDelaySlots[trg] != 1) {
		os << "(spkQuePtr" << model.neuronName[trg] << " * " << model.neuronN[trg] << ") + ";
	    }
	    os << "j]] = gFunc" << model.synapseName[k] << "(grawp" << model.synapseName[k] << "[n * ";
	    os << model.neuronN[trg] << " + glbSpk" << model.neuronName[trg] << "[";
	    if (model.neuronDelaySlots[trg] != 1) {
		os << "(spkQuePtr" << model.neuronName[trg] << " * " << model.neuronN[trg] << ") + ";
	    }
	    os << "j]]);" << ENDL; 
	    os << CB(131);
	    os << CB(121);
	    os << CB(910);
	}
	os << CB(811);
    }
    os << ENDL;
    os << "#endif" << ENDL;
    os.close();
}
