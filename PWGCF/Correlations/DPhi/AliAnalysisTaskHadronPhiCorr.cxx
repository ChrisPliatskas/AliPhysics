/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

#include "TChain.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "THnSparse.h"
#include "TParticle.h"
#include "TStopwatch.h"

#include "AliAnalysisTask.h"
#include "AliAnalysisManager.h"

#include "AliESDEvent.h"
#include "AliESDInputHandler.h"
#include "AliESDtrackCuts.h"
#include "AliAODEvent.h"
#include "AliAODHandler.h"
#include "AliAODMCParticle.h"
#include "AliCFParticle.h"

#include "AliEventPoolManager.h"
#include "AliMultSelection.h"

#include "AliPID.h"
#include "AliESDpid.h"
#include "AliAODPid.h"
#include "AliPIDResponse.h"

#include "AliMCEvent.h"
//#include "AliStack.h"
#include "AliMCEventHandler.h"

#include "AliAnalysisTaskHadronPhiCorr.h"

using std::cout;
using std::endl;

ClassImp(AliAnalysisTaskHadronPhiCorr)
//________________________________________________________________________
AliAnalysisTaskHadronPhiCorr::AliAnalysisTaskHadronPhiCorr(const char *name, Bool_t isHH, Float_t multLow, Float_t multHigh)
: AliAnalysisTaskSE(name),
fVevent(0),
fPoolMgr(0x0),
fLSPoolMgr(0x0),
fTruePoolMgr(0x0),
fHHPoolMgr(0x0),
fESD(0),
fAOD(0),
fpidResponse(0),
fOutputList(0),
fNevents(0),
fNumTracks(0),
fphiEff(0),
fhEff(0),
ftrigEff(0),
fphiEffHist(0),
fhEffHist(0),
ftrigEffHist(0),
fVtxZ(0),
fVtxX(0),
fVtxY(0),
fVtxZmixbins(0),
fTrigMulti(0),
fTrkPt(0),
fTrketa(0),
fTrkphi(0),
fHybridTrkPt(0),
fHybridTrketa(0),
fHybridTrkphi(0),
fHybridGlobalTrkPt(0),
fHybridGlobalTrketa(0),
fHybridGlobalTrkphi(0),
fdEdx(0),
fTPCNpts(0),
fHadronPT(0),
fHadronTrigPT(0),
fHadronTrigPhiPT(0),
fKaonDist(0),
fKaonPID(0),
fKKUSDist(0),
fKKLSDist(0),
fKKUSTrigDist(0),
fKKLSTrigDist(0),
fkplusPerEvent(0),
fkminusPerEvent(0),
fLSpairsPerEvent(0),
fUSpairsPerEvent(0),
fTrigDist(0),
fTrigPerEvent(0),
fTrigSameUSDist(0),
fTrigSameLSDist(0),
fTrigHHDist(0),
fLSMixStatZVtx(0),
fLSMixTrackStatZVtx(0),
fLSNoMixEvents(0),
fUSMixStatZVtx(0),
fUSMixTrackStatZVtx(0),
fUSNoMixEvents(0),
fHHMixStatZVtx(0),
fHHMixTrackStatZVtx(0),
fHHNoMixEvents(0),
fTruePhiDist(0),
fTrueHDist(0),
fTruePrimHDist(0),
fTrueNotPrimHDist(0),
fTrueSecHDist(0),
fTriggeredTruePhiDist(0),
fTriggeredTrueHDist(0),
fTriggeredTruePrimHDist(0),
fTriggeredTrueSecHDist(0),
fTriggeredTrueNotPrimHDist(0),
fHadronsVsPhi(0),
fHadronsVsPhiPairs(0),
fRatio(0),
fPairRatio(0),
fDphiHPhi(0),
fDphiTrueHPhi(0),
fDphiTrueHPhiMixed(0),
fDphiTrueAcceptanceHPhi(0),
fDphiHKK(0),
fDphiHPhiMixed(0),
fDphiHKKMixed(0),
fDphiHH(0),
fDphiHHMixed(0)
{
    // Constructor
    // Define input and output slots here
    // Input slot #0 works with a TChain
    DefineInput(0, TChain::Class());
    // Output slot #0 id reserved by the base class for AOD
    // Output slot #1 writes into a TH1 container
    DefineOutput(1, TList::Class());
    //printf("\n\n!!!!!!!!!!]\n done with the constructor! \n");
    //fflush(stdout);
    IS_HH = isHH;
    MULT_LOW = multLow;
    MULT_HIGH = multHigh;

    IS_MC_TRUE = kFALSE;
    IS_MC_KAON = kFALSE;
    IS_MC_KTRACK = kFALSE;
    USE_ACCPT = kFALSE;

    DO_SINGLE_TRIGGER = kFALSE;
    SELECT_TRIGGER_EVENTS = kFALSE;
    DO_HIGHEST_TRIGGER = kFALSE;
    DO_PER_EVENT_SCALING = kFALSE;

    KAON_ETA_CUT = 0.8;
    KAON_TPC_CUT = 3.0;
    KAON_TOF_CUT = 3.0;
    IS_KAON_TOF_VETO = kFALSE;
    KAON_TRK_BIT = AliAODTrack::kTrkGlobalNoDCA;

    TRIG_TRK_BIT = AliAODTrack::kIsHybridGCG;
    ASSOC_TRK_BIT = AliAODTrack::kTrkGlobalNoDCA;

    Z_VTX_MIN = -10.0;
    Z_VTX_MAX = 10.0;
    Z_VTX_NBINS = 10;

    CENT_ESTIMATOR = "V0A";

    fDphiHPhi = new THnSparseF*[Z_VTX_NBINS];
    fDphiTrueHPhi = new THnSparseF*[Z_VTX_NBINS];
    fDphiTrueHPhiMixed = new THnSparseF*[Z_VTX_NBINS];
    fDphiTrueAcceptanceHPhi = new THnSparseF*[Z_VTX_NBINS];
    fDphiHKK = new THnSparseF*[Z_VTX_NBINS];
    fDphiHPhiMixed = new THnSparseF*[Z_VTX_NBINS];
    fDphiHKKMixed = new THnSparseF*[Z_VTX_NBINS];
    fDphiHH = new THnSparseF*[Z_VTX_NBINS];
    fDphiHHMixed = new THnSparseF*[Z_VTX_NBINS];
}
//________________________________________________________________________
AliAnalysisTaskHadronPhiCorr::AliAnalysisTaskHadronPhiCorr()
: AliAnalysisTaskSE("DefaultTask_hPhiCorr"),
fVevent(0),
fPoolMgr(0x0),
fLSPoolMgr(0x0),
fTruePoolMgr(0x0),
fHHPoolMgr(0x0),
fESD(0),
fAOD(0),
fpidResponse(0),
fOutputList(0),
fNevents(0),
fNumTracks(0),
fphiEff(0),
fhEff(0),
ftrigEff(0),
fphiEffHist(0),
fhEffHist(0),
ftrigEffHist(0),
fVtxZ(0),
fVtxX(0),
fVtxY(0),
fVtxZmixbins(0),
fTrigMulti(0),
fTrkPt(0),
fTrketa(0),
fTrkphi(0),
fHybridTrkPt(0),
fHybridTrketa(0),
fHybridTrkphi(0),
fHybridGlobalTrkPt(0),
fHybridGlobalTrketa(0),
fHybridGlobalTrkphi(0),
fdEdx(0),
fTPCNpts(0),
fHadronPT(0),
fHadronTrigPT(0),
fHadronTrigPhiPT(0),
fKaonPID(0),
fKaonDist(0),
fKKUSDist(0),
fKKLSDist(0),
fKKUSTrigDist(0),
fKKLSTrigDist(0),
fkplusPerEvent(0),
fkminusPerEvent(0),
fLSpairsPerEvent(0),
fUSpairsPerEvent(0),
fTrigDist(0),
fTrigPerEvent(0),
fTrigSameUSDist(0),
fTrigSameLSDist(0),
fTrigHHDist(0),
fLSMixStatZVtx(0),
fLSMixTrackStatZVtx(0),
fLSNoMixEvents(0),
fUSMixStatZVtx(0),
fUSMixTrackStatZVtx(0),
fUSNoMixEvents(0),
fHHMixStatZVtx(0),
fHHMixTrackStatZVtx(0),
fHHNoMixEvents(0),
fTruePhiDist(0),
fTrueHDist(0),
fTruePrimHDist(0),
fTrueNotPrimHDist(0),
fTrueSecHDist(0),
fTriggeredTruePhiDist(0),
fTriggeredTrueHDist(0),
fTriggeredTruePrimHDist(0),
fTriggeredTrueSecHDist(0),
fTriggeredTrueNotPrimHDist(0),
fHadronsVsPhi(0),
fHadronsVsPhiPairs(0),
fRatio(0),
fPairRatio(0),
fDphiHPhi(0),
fDphiTrueHPhi(0),
fDphiTrueHPhiMixed(0),
fDphiTrueAcceptanceHPhi(0),
fDphiHKK(0),
fDphiHPhiMixed(0),
fDphiHKKMixed(0),
fDphiHH(0),
fDphiHHMixed(0)
{
    //Default constructor
    // Define input and output slots here
    // Input slot #0 works with a TChain
    DefineInput(0, TChain::Class());
    // Output slot #0 id reserved by the base class for AOD
    // Output slot #1 writes into a TH1 container
    // DefineOutput(1, TH1I::Class());
    DefineOutput(1, TList::Class());
    //DefineOutput(3, TTree::Class());

    IS_MC_TRUE = kFALSE;
    IS_MC_KAON = kFALSE;
    IS_MC_KTRACK = kFALSE;
    USE_ACCPT = kFALSE;

    DO_SINGLE_TRIGGER = kFALSE;
    SELECT_TRIGGER_EVENTS = kFALSE;
    DO_HIGHEST_TRIGGER = kFALSE;
    DO_PER_EVENT_SCALING = kFALSE;

    IS_HH = kFALSE;
    MULT_LOW = 0.0;
    MULT_HIGH = 100.0;

    KAON_ETA_CUT = 0.8;
    KAON_TPC_CUT = 3.0;
    KAON_TOF_CUT = 3.0;
    IS_KAON_TOF_VETO = kFALSE;
    KAON_TRK_BIT = AliAODTrack::kTrkGlobalNoDCA;

    TRIG_TRK_BIT = AliAODTrack::kIsHybridGCG;
    ASSOC_TRK_BIT = AliAODTrack::kTrkGlobalNoDCA;

    Z_VTX_MIN = -10.0;
    Z_VTX_MAX = 10.0;
    Z_VTX_NBINS = 10;

    CENT_ESTIMATOR = "V0A";

    fDphiHPhi = new THnSparseF*[Z_VTX_NBINS];
    fDphiTrueHPhi = new THnSparseF*[Z_VTX_NBINS];
    fDphiTrueAcceptanceHPhi = new THnSparseF*[Z_VTX_NBINS];
    fDphiTrueHPhiMixed = new THnSparseF*[Z_VTX_NBINS];
    fDphiHKK = new THnSparseF*[Z_VTX_NBINS];
    fDphiHPhiMixed = new THnSparseF*[Z_VTX_NBINS];
    fDphiHKKMixed = new THnSparseF*[Z_VTX_NBINS];
    fDphiHH = new THnSparseF*[Z_VTX_NBINS];
    fDphiHHMixed = new THnSparseF*[Z_VTX_NBINS];

}
//________________________________________________________________________
AliAnalysisTaskHadronPhiCorr::~AliAnalysisTaskHadronPhiCorr()
{
    //Destructor
    delete fOutputList;
    delete fDphiHPhi;
    delete fDphiHPhiMixed;
    delete fDphiHKKMixed;
    delete fDphiHH;
    delete fDphiHHMixed;
    delete fDphiTrueHPhi;
    delete fDphiTrueHPhiMixed;
    delete fDphiTrueAcceptanceHPhi;

    delete fPoolMgr;
    delete fLSPoolMgr;
    delete fHHPoolMgr;
}
//________________________________________________________________________
void AliAnalysisTaskHadronPhiCorr::LoadEfficiencies(TFile* filename){
    //TFile* effFile = TFile::Open(filename.Data());
    TFile* effFile = filename;
    //TFile* effFile = TFile::Open("/home/alidock/alirepos/utaustin/efficiency/fits_17f2bCENTTPC80efficiency.root");
   /* if(!effFile){
        printf("\n\n\nNo Efficiency File!!!\n\n\n");
        AliFatal(Form("No Efficiency file was found at %s!", filename.Data()));
    }
   */
    
    fphiEff = (TF1*)(effFile->Get("phiFit")->Clone("fphiEff"));
    if(!fphiEff){
        AliFatal("No phi Eff found!!");
    }

    fhEff = (TF1*)(effFile->Get("hFit")->Clone("fhEff"));
    if(!fhEff){
        AliFatal("No h Eff found!!");
    }
    
    ftrigEff = (TF1*)(effFile->Get("trigFit")->Clone("ftrigEff"));
    if(!ftrigEff){
        AliFatal("No trig Eff found!!");
    }
    
    //printf("Testing Efficiencies before close: %f\n\n", fphiEff->Eval(3.21017));*/
    //effFile->Close("R");
    //printf("Testing Efficiencies after close: %f\n\n", fphiEff->Eval(3.21017));

}

void AliAnalysisTaskHadronPhiCorr::LoadEfficiencies(TF1* phieff, TF1* heff, TF1* trigeff){
    
    fphiEff = (TF1*)phieff->Clone("phiEff");
    fhEff = (TF1*)heff->Clone("hEff"); 
    ftrigEff = (TF1*)trigeff->Clone("trigEff");
    
    //work-around for error when getting eff from remote file?
/*
    fphiEff = new TF1("fphiEff2", "pol3(0)", 0.5, 20.);
    fhEff = new TF1("fhEff2", "pol5(0)", 0.5, 20.);
    ftrigEff = new TF1("ftrigEff2", "pol3(0)", 0.5,  20.);

    fphiEff->SetParameters(phieff->GetParameter(0), phieff->GetParameter(1), phieff->GetParameter(2), phieff->GetParameter(3));
    fhEff->SetParameters(heff->GetParameter(0), heff->GetParameter(1), heff->GetParameter(2), heff->GetParameter(3), heff->GetParameter(4), heff->GetParameter(5));
    ftrigEff->SetParameters(trigeff->GetParameter(0), trigeff->GetParameter(1), trigeff->GetParameter(2), trigeff->GetParameter(3));

    printf("eff test: %f, %f, %f\n", fphiEff->Eval(2.0), fhEff->Eval(2.0), ftrigEff->Eval(2.0));
*/
}

void AliAnalysisTaskHadronPhiCorr::LoadEfficiencyHistos(TFile* effFile){
    
    fphiEffHist = (TH1D*)(effFile->Get("phiPTEff")->Clone("fphiEffHist"));
    if(!fphiEffHist){
        AliFatal("No phi Eff found!!");
    }

    fhEffHist = (TH1D*)(effFile->Get("hadronPTEff")->Clone("fhEffHist"));
    if(!fhEffHist){
        AliFatal("No h Eff found!!");
    }
    
    ftrigEffHist = (TH1D*)(effFile->Get("trigEff")->Clone("ftrigEffHist"));
    if(!ftrigEffHist){
        AliFatal("No trig Eff found!!");
    }

    printf("test efficiency: %f\n", fphiEffHist->GetBinContent(fphiEffHist->GetXaxis()->FindBin(2.1)));

}

//________________________________________________________________________
void AliAnalysisTaskHadronPhiCorr::UserCreateOutputObjects()
{
    // Create histograms
    // Called once
    AliDebug(3, "Creating Output Objects");
    
    /////////////////////////////////////////////////
    //Automatic determination of the analysis mode//
    ////////////////////////////////////////////////
    AliVEventHandler *inputHandler = dynamic_cast<AliVEventHandler *>(AliAnalysisManager::GetAnalysisManager()->GetInputEventHandler());
    if(!TString(inputHandler->IsA()->GetName()).CompareTo("AliAODInputHandler")){
        SetAODAnalysis();
    } else {
        SetESDAnalysis();
    }
    printf("Analysis Mode: %s Analysis\n", IsAODanalysis() ? "AOD" : "ESD");
   
    ////////////////////////////
    // Set-up for Mixed Event //
    ////////////////////////////

    Int_t poolSize = 500;
    Int_t trackDepth = 1000;

    //Int_t numVtxZBins = 10;
    //Double_t vtxZBins[11] = {-10.0, -6.15, -3.90, -2.13, -0.59, 0.86, 2.29, 3.77, 5.39, 7.30, 10.0};
    //Double_t vtxZBins[11] = {-10.0, -8.0, -6.0, -4.0, -2.0, 0.0, 2.0, 4.0, 6.0, 8.0, 10.0};
    
    Double_t vtxZBins[Z_VTX_NBINS+1];
    Double_t vtxStepSize = (Z_VTX_MAX - Z_VTX_MIN)/Double_t(Z_VTX_NBINS);
    for(int ivtx = 0; ivtx < Z_VTX_NBINS+1; ivtx++){
        vtxZBins[ivtx] = Z_VTX_MIN + ivtx*vtxStepSize;
        if(vtxZBins[ivtx] > Z_VTX_MAX){
            vtxZBins[ivtx] = Z_VTX_MAX;
        }
    }

    Int_t numMultBins = 1;
    Double_t multBins[2] = {MULT_LOW, MULT_HIGH};

    fPoolMgr = new AliEventPoolManager(poolSize, trackDepth, numMultBins, multBins, Z_VTX_NBINS, vtxZBins);
    fPoolMgr->SetTargetValues(trackDepth, 0.1, 5);
    fLSPoolMgr = new AliEventPoolManager(poolSize, trackDepth, numMultBins, multBins, Z_VTX_NBINS, vtxZBins);
    fLSPoolMgr->SetTargetValues(trackDepth, 0.1, 5);
    fHHPoolMgr = new AliEventPoolManager(poolSize, trackDepth, numMultBins, multBins, Z_VTX_NBINS, vtxZBins);
    fHHPoolMgr->SetTargetValues(trackDepth, 0.1, 5);
    fTruePoolMgr = new AliEventPoolManager(poolSize, trackDepth/10.0, numMultBins, multBins, Z_VTX_NBINS, vtxZBins);
    fTruePoolMgr->SetTargetValues(trackDepth/10.0, 0.1, 5);


    ////////////////
    //Output list//
    ///////////////
    fOutputList = new TList();
    fOutputList->SetOwner();
    
    fNevents = new TH1F("fNevents","No of events",3,-0.5,2.5);
    fOutputList->Add(fNevents);
    fNevents->GetYaxis()->SetTitle("counts");
    fNevents->GetXaxis()->SetBinLabel(1,"All");
    fNevents->GetXaxis()->SetBinLabel(2,"With >2 Trks");
    fNevents->GetXaxis()->SetBinLabel(3,"Vtx_{z}<10cm");

    fNumTracks = new TH1F("fNumTracks", "Number of Tracks/evt", 1000, 0, 1000);
    fOutputList->Add(fNumTracks);
    
    fVtxZ = new TH1F("fVtxZ","Z vertex position;Vtx_{z};counts",1000,-50,50);
    fOutputList->Add(fVtxZ);
    
    fVtxY = new TH1F("fVtxY","Y vertex position;Vtx_{y};counts",1000,-50,50);
    fOutputList->Add(fVtxY);
    
    fVtxX = new TH1F("fVtxX","X vertex position;Vtx_{x};counts",1000,-50,50);
    fOutputList->Add(fVtxX);
    
    fVtxZmixbins = new TH1F("fVtxZmixbins", "Z vertex position (Mixing Bins);Vtx_{z};counts", Z_VTX_NBINS, Z_VTX_MIN, Z_VTX_MAX);
    fOutputList->Add(fVtxZmixbins);

    fTrigMulti = new TH2F("fTrigMulti","Multiplicity distribution for different triggers; Trigger type; multiplicity",11,-1,10,2000,0,2000);
    fOutputList->Add(fTrigMulti);
    
    //Global track histos
    fTrkPt = new TH1F("fTrkPt","p_{T} distribution of all tracks;p_{T} (GeV/c);counts",1000,0,100);
    fOutputList->Add(fTrkPt);
    
    fTrketa = new TH1F("fTrketa","All Track #eta distribution;#eta;counts",100,-2.0,2.0);
    fOutputList->Add(fTrketa);
    
    fTrkphi = new TH1F("fTrkphi","All Track #phi distribution;#phi;counts",100,0,6.3);
    fOutputList->Add(fTrkphi);

    //HybridTPC track histos
    fHybridTrkPt = new TH1F("fHybridTrkPt","p_{T} distribution of all hybrid tracks;p_{T} (GeV/c);counts",1000,0,100);
    fOutputList->Add(fHybridTrkPt);
    
    fHybridTrketa = new TH1F("fHybridTrketa","All Hybrid Track #eta distribution;#eta;counts",100,-2.0,2.0);
    fOutputList->Add(fHybridTrketa);
    
    fHybridTrkphi = new TH1F("fHybridTrkphi","All Hybrid Track #phi distribution;#phi;counts",100,0,6.3);
    fOutputList->Add(fHybridTrkphi);
    
    //HybridGlobal track histos
    fHybridGlobalTrkPt = new TH1F("fHybridGlobalTrkPt","p_{T} distribution of all hybrid tracks;p_{T} (GeV/c);counts",1000,0,100);
    fOutputList->Add(fHybridGlobalTrkPt);
    
    fHybridGlobalTrketa = new TH1F("fHybridGlobalTrketa","All HybridGlobal Track #eta distribution;#eta;counts",100,-2.0,2.0);
    fOutputList->Add(fHybridGlobalTrketa);
    
    fHybridGlobalTrkphi = new TH1F("fHybridGlobalTrkphi","All HybridGlobal Track #phi distribution;#phi;counts",100,0,6.3);
    fOutputList->Add(fHybridGlobalTrkphi);
 
    fdEdx = new TH2F("fdEdx","All Track dE/dx distribution;p (GeV/c);dE/dx",200,0,20,500,0,160);
    fOutputList->Add(fdEdx);
    
    fTPCNpts = new TH2F("fTPCNpts","All track TPC Npoints used for dE/dx calculation;p (GeV/c);N points",200,0,20,200,0.,200.);
    fOutputList->Add(fTPCNpts);

    // Kaon distribution and PID histograms

    fKaonDist = new TH3F("fKaonDist", "Kaon Distribution;p_{T} (GeV/c);#varphi;#eta", 75, 0.0, 15.0, 32, -3.1416, 3.1416, 60, -3.0, 3.0); 
    fOutputList->Add(fKaonDist);

    fKaonPID = new TH3F("fKaonPID", "Kaon PID;p_{T} (GeV/c);n#sigma_{TPC};n#sigma_{TOF}", 75, 0.0, 15.0, 50, -5.0, 5.0, 50, -5.0, 5.0);
    fOutputList->Add(fKaonPID);
    
    // Histogram for trigger distribution
    Int_t trigBins[3] = {200,64,40};
    Double_t trigMin[3] = {0.1, 0.0, -2.0};
    Double_t trigMax[3] = {20.1, 6.28, 2.0};

    fTrigDist = new THnSparseF("fTrigDist", "Distribution for trigger particles", 3, trigBins, trigMin, trigMax);
    fTrigDist->Sumw2();
    fOutputList->Add(fTrigDist);

    fTrigPerEvent = new TH1D("fTrigPerEvent", "Number of Trigger particles in 4 < p_{T} < 8 GeV/c per Event", 10, 0, 10);
    fOutputList->Add(fTrigPerEvent);

     //Trigger Distribution for doing trigger particle scaling (same and mixed, hadron triggers for US or LS pairs are separate) 
    fTrigSameUSDist = new TH2D("fTrigSameUSDist", "Trigger count for same event, US pairs;p_{T}^{trig};Vtx_{z}", 18, 2.0, 20.0, Z_VTX_NBINS, Z_VTX_MIN, Z_VTX_MAX);
    fOutputList->Add(fTrigSameUSDist);

    fTrigSameLSDist = new TH2D("fTrigSameLSDist", "Trigger count for same event, LS pairs;p_{T}^{trig};Vtx_{z}", 18, 2.0, 20.0, Z_VTX_NBINS, Z_VTX_MIN, Z_VTX_MAX);
    fOutputList->Add(fTrigSameLSDist);

    fTrigHHDist = new TH2D("fTrigHHDist", "Trigger count for same event, h-h correlations;p_{T}^{trig};Vtx_{z}", 18, 2.0, 20.0, Z_VTX_NBINS, Z_VTX_MIN, Z_VTX_MAX);
    fOutputList->Add(fTrigHHDist);

    //Histograms for Mixed Event Stats
    fLSMixStatZVtx = new TH2D("fLSMixStatZVtx", "LS Mixed Event Statistics;NEvent in pool;Vtx_z", 100, 1, 1001, Z_VTX_NBINS, vtxZBins);
    fOutputList->Add(fLSMixStatZVtx);

    fLSMixTrackStatZVtx = new TH2D("fLSMixtrackStatZVtx", "LS Mixed Event Statistics;NTracks in pool;Vtx_z", 200, 1, 5001, Z_VTX_NBINS, vtxZBins);
    fOutputList->Add(fLSMixTrackStatZVtx);

    fLSNoMixEvents = new TH1D("fLSNoMixEvents", "Number of LS Mixed Events", 1, -0.5, 0.5);
    fOutputList->Add(fLSNoMixEvents);

    fUSMixStatZVtx = new TH2D("fUSMixStatZVtx", "US Mixed Event Statistics;NEvent in pool;Vtx_z", 100, 1, 1001, Z_VTX_NBINS, vtxZBins);
    fOutputList->Add(fUSMixStatZVtx);

    fUSMixTrackStatZVtx = new TH2D("fUSMixtrackStatZVtx", "US Mixed Event Statistics;NTracks in pool;Vtx_z", 200, 1, 5001, Z_VTX_NBINS, vtxZBins);
    fOutputList->Add(fUSMixTrackStatZVtx);

    fUSNoMixEvents = new TH1D("fUSNoMixEvents", "Number of US Mixed Events", 1, -0.5, 0.5);
    fOutputList->Add(fUSNoMixEvents);

    fHHMixStatZVtx = new TH2D("fHHMixStatZVtx", "HH Mixed Event Statistics;NEvent in pool;Vtx_z", 100, 1, 1001, Z_VTX_NBINS, vtxZBins);
    fOutputList->Add(fHHMixStatZVtx);

    fHHMixTrackStatZVtx = new TH2D("fHHMixtrackStatZVtx", "HH Mixed Event Statistics;NTracks in pool;Vtx_z", 200, 1, 5001, Z_VTX_NBINS, vtxZBins);
    fOutputList->Add(fHHMixTrackStatZVtx);

    fHHNoMixEvents = new TH1D("fHHNoMixEvents", "Number of HH Mixed Events", 1, -0.5, 0.5);
    fOutputList->Add(fHHNoMixEvents);

    
    // Additional Histograms for US and LS Kaon pairs:
    Int_t bins[5] = {100, 80, 32, 40, 40}; //pt, invmass, phi, eta, y
    Double_t min[5] = {0.0, 0.99, 0, -2.0, -2.0};
    Double_t max[5] = {10.0, 1.07, 6.28, 2.0, 2.0};
 
    fKKUSDist = new THnSparseF("fkkUSDist", "Distribution for all US Kaon pairs", 5, bins, min, max);
    fKKUSDist->Sumw2();
    fOutputList->Add(fKKUSDist);

    fKKLSDist = new THnSparseF("fkkLSDist", "Distribution for all LS Kaon pairs", 5, bins, min, max);
    fKKLSDist->Sumw2();
    fOutputList->Add(fKKLSDist);

    fKKUSTrigDist = new THnSparseF("fkkUSTrigDist", "Distribution for all US Kaon pairs in Events with Trigger", 5, bins, min, max);
    fKKUSTrigDist->Sumw2();
    fOutputList->Add(fKKUSTrigDist);

    fKKLSTrigDist = new THnSparseF("fkkLSTrigDist", "Distribution for all LS Kaon pairs in Events with Trigger", 5, bins, min, max);
    fKKLSTrigDist->Sumw2();
    fOutputList->Add(fKKLSTrigDist);
    
    fkplusPerEvent = new TH1D("fkplusPerEvent", "K^{+} per Event", 100, 0, 100);
    fOutputList->Add(fkplusPerEvent);

    fkminusPerEvent = new TH1D("fkminusPerEvent", "K^{-} per Event", 100, 0, 100);
    fOutputList->Add(fkminusPerEvent);

    fLSpairsPerEvent = new TH1D("fLSpairsPerEvent", "LS KK pairs per event", 100, 0, 100);
    fOutputList->Add(fLSpairsPerEvent);

    fUSpairsPerEvent = new TH1D("fUSpairsPerEvent", "US KK pairs per event", 100, 0, 100);
    fOutputList->Add(fUSpairsPerEvent); 
 
    // Hadron pT histograms for different events (min bias, trigger present, trigger and phi candidate present)

    fHadronPT = new TH1D("fHadronPT", "Hadron p_{T} for min bias Events", 100, 0., 10.0);
    fHadronPT->Sumw2();
    fOutputList->Add(fHadronPT);

    fHadronTrigPT = new TH1D("fHadronTrigPT", "Hadron p_{T} for events with high pT trigger", 100, 0., 10.0);
    fHadronTrigPT->Sumw2();
    fOutputList->Add(fHadronTrigPT);

    fHadronTrigPhiPT = new TH1D("fHadronTrigPhiPT", "Hadron p_{T} for events with high pT trigger and phi candidate", 100, 0., 10.0);
    fHadronTrigPhiPT->Sumw2();
    fOutputList->Add(fHadronTrigPhiPT);

    if(IS_MC_TRUE){
        //  distribution for true phi mesons
        fTruePhiDist = new THnSparseF("fTruePhiDist", "Distribution for true #phi mesons", 5, bins, min, max);
        fTruePhiDist->Sumw2();
        fOutputList->Add(fTruePhiDist);

        Int_t hbins[] = {100, 32, 40, 40, 5};
        Double_t hmin[] = {0.0, 0.0, -2.0, -2.0, 1};
        Double_t hmax[] = {10.0, 2.0*TMath::Pi(), 2.0, 2.0, 6};
        //  distribtion for true hadrons (pi, K, p, e, mu)
        fTrueHDist = new THnSparseF("fTrueHDist", "Distribution for true hadrons", 5, hbins, hmin, hmax);
        fTrueHDist->Sumw2();
        fOutputList->Add(fTrueHDist);

        fTruePrimHDist = new THnSparseF("fTruePrimHDist", "Distribution for true physical primary hadrons", 5, hbins, hmin, hmax);
        fTruePrimHDist->Sumw2();
        fOutputList->Add(fTruePrimHDist);

        fTrueNotPrimHDist = new THnSparseF("fTrueNotPrimHDist", "Distribution for true physical primary hadrons", 5, hbins, hmin, hmax);
        fTrueNotPrimHDist->Sumw2();
        fOutputList->Add(fTrueNotPrimHDist);

        fTrueSecHDist = new THnSparseF("fTrueSecHDist", "Distribution for true secondary weak decay hadrons", 5, hbins, hmin, hmax);
        fTrueSecHDist->Sumw2();
        fOutputList->Add(fTrueSecHDist);

        //  distribution for true hadrons from events with at least 1 trigger particle

        fTriggeredTruePhiDist = new THnSparseF("fTriggeredTruePhiDist", "Distribution for triggered true #phi mesons", 5, bins, min, max);
        fTriggeredTruePhiDist->Sumw2();
        fOutputList->Add(fTriggeredTruePhiDist);

        fTriggeredTrueHDist = new THnSparseF("fTriggeredTrueHDist", "Distribution for triggered true hadrons", 5, hbins, hmin, hmax);
        fTriggeredTrueHDist->Sumw2();
        fOutputList->Add(fTriggeredTrueHDist);

        fTriggeredTruePrimHDist = new THnSparseF("fTriggeredTruePrimHDist", "Distribution for triggered true physical primary hadrons", 5, hbins, hmin, hmax);
        fTriggeredTruePrimHDist->Sumw2();
        fOutputList->Add(fTriggeredTruePrimHDist);
 
        fTriggeredTrueNotPrimHDist = new THnSparseF("fTriggeredTrueNotPrimHDist", "Distribution for triggered true NOT physical primary hadrons", 5, hbins, hmin, hmax);
        fTriggeredTrueNotPrimHDist->Sumw2();
        fOutputList->Add(fTriggeredTrueNotPrimHDist);
        
        fTriggeredTrueSecHDist = new THnSparseF("fTriggeredTrueSecHDist", "Distribution for triggered true secondary weak decay hadrons", 5, hbins, hmin, hmax);
        fTriggeredTrueSecHDist->Sumw2();
        fOutputList->Add(fTriggeredTrueSecHDist);

        fHadronsVsPhi = new TH2D("fHadronsVsPhi", "Number of Hadrons vs number of #phi in acceptance", 100, 0, 100, 100, 0, 100);
        fHadronsVsPhi->Sumw2();
        fOutputList->Add(fHadronsVsPhi);

        fHadronsVsPhiPairs = new TH2D("fHadronsVsPhiPairs", "Number of trigger-Hadron pairs vs number of trigger-#phi paris in acceptance", 100, 0, 100, 100, 0, 100);
        fHadronsVsPhiPairs->Sumw2();
        fOutputList->Add(fHadronsVsPhiPairs);

        fRatio = new TH1D("fRatio", "phi/hadrons in acceptance per event", 2000, 0, 2);
        fRatio->Sumw2();
        fOutputList->Add(fRatio);

        fPairRatio = new TH1D("fPairRatio", "h-phi/h-h in acceptance per event", 2000, 0, 2);
        fPairRatio->Sumw2();
        fOutputList->Add(fPairRatio);

    }


    // Delta-phi histograms for different hadron-particle correlations (trigger pT, correlation pT, delta-phi, delta-eta, inv mass)
    Int_t dphi_bins[5]=    {10,   18,   16,    20,  80};
    Double_t dphi_min[5] = { 2.0,  1.0, -1.0*TMath::Pi()/2.0, -2.0, 0.99};
    Double_t dphi_max[5] = {12.0, 10.0,  3.0*TMath::Pi()/2.0,  2.0, 1.07};

    for(int izvtx = 0; izvtx < Z_VTX_NBINS; izvtx++){

        if(IS_MC_TRUE){
            fDphiTrueHPhi[izvtx] = new THnSparseF(Form("fDphiTrueHPhiz%i", izvtx), "MC True Hadron-#phi Correlations", 5, dphi_bins, dphi_min, dphi_max);
            fDphiTrueHPhi[izvtx]->Sumw2();
            fOutputList->Add(fDphiTrueHPhi[izvtx]);

            fDphiTrueAcceptanceHPhi[izvtx] = new THnSparseF(Form("fDphiTrueAcceptanceHPhiz%i", izvtx), "MC TrueAcceptance Hadron-#phi Correlations", 5, dphi_bins, dphi_min, dphi_max);
            fDphiTrueAcceptanceHPhi[izvtx]->Sumw2();
            fOutputList->Add(fDphiTrueAcceptanceHPhi[izvtx]);


            fDphiTrueHPhiMixed[izvtx] = new THnSparseF(Form("fDphiTrueHPhiMixedz%i", izvtx), "True Hadron-#phi #Delta#phi mixed event Correlations", 5, dphi_bins, dphi_min, dphi_max);
            fDphiTrueHPhiMixed[izvtx]->Sumw2();
            fOutputList->Add(fDphiTrueHPhiMixed[izvtx]);

        }else{
            fDphiHPhi[izvtx] = new THnSparseF(Form("fDphiHPhiz%i", izvtx), "Hadron-#phi #Delta#phi correlations", 5, dphi_bins, dphi_min, dphi_max);
            fDphiHPhi[izvtx]->Sumw2();
            fOutputList->Add(fDphiHPhi[izvtx]);

            fDphiHKK[izvtx] = new THnSparseF(Form("fDphiHKKz%i", izvtx), "Hadron-#KK likesign #Delta#phi correlations", 5, dphi_bins, dphi_min, dphi_max);
            fDphiHKK[izvtx]->Sumw2();
            fOutputList->Add(fDphiHKK[izvtx]);

            fDphiHPhiMixed[izvtx] = new THnSparseF(Form("fDphiHPhiMixedz%i", izvtx), "Hadron-#phi #Delta#phi mixed event Correlations", 5, dphi_bins, dphi_min, dphi_max);
            fDphiHPhiMixed[izvtx]->Sumw2();
            fOutputList->Add(fDphiHPhiMixed[izvtx]);

            fDphiHKKMixed[izvtx] = new THnSparseF(Form("fDphiHKKMixedz%i", izvtx), "Hadron-#KK likesign #Delta#phi mixed event Correlations", 5, dphi_bins, dphi_min, dphi_max);
            fDphiHKKMixed[izvtx]->Sumw2();
            fOutputList->Add(fDphiHKKMixed[izvtx]);
        }

        fDphiHH[izvtx] = new THnSparseF(Form("fDphiHHz%i", izvtx), "Hadron-Hadron correlations", 4, dphi_bins, dphi_min, dphi_max);
        fDphiHH[izvtx]->Sumw2();
        fOutputList->Add(fDphiHH[izvtx]);

        fDphiHHMixed[izvtx] = new THnSparseF(Form("fDphiHHMixedz%i", izvtx), "Hadron-Hadron mixed event correlations", 4, dphi_bins, dphi_min, dphi_max);
        fDphiHHMixed[izvtx]->Sumw2();
        fOutputList->Add(fDphiHHMixed[izvtx]); 
    }

    PostData(1,fOutputList);

    printf("Finished Create Output Objects\n");
}


//___________________________________________________________________________
Bool_t AliAnalysisTaskHadronPhiCorr::MakeCorrelations(Int_t triggerIndex,const AliHContainer& trigger, const std::vector<AliPhiContainer>& phiVec, THnSparse *fDphi, Double_t zVtx, Int_t numtrigs = 1){

    Double_t dphi_point[5];
    AliPhiContainer phi;
    //for MC true case, change triggerIndex to trigger stack position
    if(IS_MC_TRUE || IS_MC_KAON){
        AliVParticle *vpart = dynamic_cast<AliVParticle*>(fVevent->GetTrack(triggerIndex));
        AliAODTrack *aodtrack = dynamic_cast<AliAODTrack*>(vpart);
        Int_t tracklabel = aodtrack->GetLabel();
        if(tracklabel < 0) tracklabel = -99;
        triggerIndex = tracklabel;
    }

    /*for(int iphi = 0; iphi < phiVec.size(); iphi++){
        phi = phiVec[iphi];
        if(triggerIndex == phi.daughter1TrackNum || triggerIndex == phi.daughter2TrackNum) return kTRUE; //skip if trigger hadron is one of the daughter particles
    }*/

    dphi_point[0] = trigger.particle.Pt();
    for(int iphi = 0; iphi < phiVec.size(); iphi++){
        phi = phiVec[iphi];
        if(triggerIndex == phi.daughter1TrackNum || triggerIndex == phi.daughter2TrackNum) continue;
        dphi_point[1] = phi.particle.Pt();
        dphi_point[2] = trigger.particle.Phi() - phi.particle.Phi();
        if(dphi_point[2] < -TMath::Pi()/2.0){
            dphi_point[2] += 2.0*TMath::Pi();
        }else if(dphi_point[2] > 3.0*TMath::Pi()/2.0){
            dphi_point[2] -= 2.0*TMath::Pi();
        }
        dphi_point[3] = trigger.particle.Eta() - phi.particle.Eta();
        //dphi_point[4] = zVtx;
        dphi_point[4] = phi.particle.M();

        Double_t weight = 1.0;
        if(!IS_MC_TRUE && !IS_MC_KAON && fphiEffHist!=0){
            if((phi.particle.Pt() > 1.0 && phi.particle.Pt() < 8.0) && trigger.particle.Pt() > 3.0 && trigger.particle.Pt() < 9.0){
                weight = 1.0/fphiEffHist->GetBinContent(fphiEffHist->GetXaxis()->FindBin((phi.particle.Pt())));
                weight = weight*(1.0/ftrigEffHist->GetBinContent(ftrigEffHist->GetXaxis()->FindBin((trigger.particle.Pt()))));
                //weight = 21.0;
            }
        }else if(IS_MC_KAON && ftrigEffHist!=0){
            weight = weight*1.0/ftrigEffHist->GetBinContent(ftrigEffHist->GetXaxis()->FindBin((trigger.particle.Pt())));
        }
        if(DO_PER_EVENT_SCALING){
            weight = weight/numtrigs;
        }
        fDphi->Fill(dphi_point, weight);
    }
    return kFALSE;
}

//___________________________________________________________________________
Bool_t AliAnalysisTaskHadronPhiCorr::MakeCorrelations(Int_t triggerIndex, AliAODMCParticle *trigger, const std::vector<AliPhiContainer>& phiVec, THnSparse *fDphi, Double_t zVtx){

    Double_t dphi_point[5];
    AliPhiContainer phi;
        
    for(int iphi = 0; iphi < phiVec.size(); iphi++){
        phi = phiVec[iphi];
        if(triggerIndex == phi.daughter1TrackNum || triggerIndex == phi.daughter2TrackNum) return kTRUE; //skip if trigger hadron is one of the daughter particles
    }

    dphi_point[0] = trigger->Pt();
    for(int iphi = 0; iphi < phiVec.size(); iphi++){
        phi = phiVec[iphi];
        dphi_point[1] = phi.particle.Pt();
        dphi_point[2] = trigger->Phi() - phi.particle.Phi();
        if(dphi_point[2] < -TMath::Pi()/2.0){
            dphi_point[2] += 2.0*TMath::Pi();
        }else if(dphi_point[2] > 3.0*TMath::Pi()/2.0){
            dphi_point[2] -= 2.0*TMath::Pi();
        }
        dphi_point[3] = trigger->Eta() - phi.particle.Eta();
        //dphi_point[4] = zVtx;
        dphi_point[4] = phi.particle.M();
        fDphi->Fill(dphi_point);
    }
    return kFALSE;
}

//___________________________________________________________________________
void AliAnalysisTaskHadronPhiCorr::MakeMixCorrelations(AliPhiContainer* phi, THnSparse *fDphiMixed, Float_t mult, Double_t zVtx, AliEventPool* fPool, Bool_t isLS){

    Double_t dphi_point[5];    
    Int_t nMix = fPool->GetCurrentNEvents();
    Int_t nTracks = 0;
    for(int iMix=0; iMix < nMix; iMix++){            
        TObjArray *tracks = fPool->GetEvent(iMix);
        tracks->SetName(Form("tracks_mult%f_z%f_tracks", mult, zVtx, tracks->GetEntriesFast()));
        Int_t numTracks = tracks->GetEntriesFast();
        nTracks+=numTracks;
        for(int ihadron = 0; ihadron < numTracks; ihadron++){
            AliCFParticle *hadron = (AliCFParticle*) tracks->At(ihadron);
            if(!hadron){
                AliFatal(Form("ERROR: Could not receive mix pool track %d\n",ihadron));
                continue;
            }
            dphi_point[0] = hadron->Pt();
            dphi_point[1] = phi->particle.Pt();
            dphi_point[2] = hadron->Phi() - phi->particle.Phi();
            if(dphi_point[2] < -TMath::Pi()/2.0){
                dphi_point[2] += 2.0*TMath::Pi();
            }else if(dphi_point[2] > 3.0*TMath::Pi()/2.0){
                dphi_point[2] -= 2.0*TMath::Pi();
            }
            dphi_point[3] = hadron->Eta() - phi->particle.Eta();
            //dphi_point[4] = zVtx;
            dphi_point[4] = phi->particle.M();

            Double_t weight = 1.0;
            if(!IS_MC_TRUE && !IS_MC_KAON && fphiEff!=0 && ftrigEff!=0){
                if((phi->particle.Pt() > 1.0 && phi->particle.Pt() < 8.0) && hadron->Pt() > 3.0){
                    weight = 1.0/fphiEffHist->GetBinContent(fphiEffHist->GetXaxis()->FindBin(phi->particle.Pt()));
                    weight = weight*(1.0/ftrigEffHist->GetBinContent(ftrigEffHist->GetXaxis()->FindBin(hadron->Pt())));
                    //weight = 21.0;
                }
            }
            fDphiMixed->Fill(dphi_point, weight);
        }
    }
    if(isLS){
        fLSMixStatZVtx->Fill(nMix, zVtx);
        fLSNoMixEvents->Fill(0);
        fLSMixTrackStatZVtx->Fill(nTracks, zVtx);
    }else{
        fUSMixStatZVtx->Fill(nMix, zVtx);
        fUSNoMixEvents->Fill(0);
        fUSMixTrackStatZVtx->Fill(nTracks, zVtx);
    }
}

//___________________________________________________________________________
void AliAnalysisTaskHadronPhiCorr::MakeHHMixCorrelations(AliCFParticle *assocPart, THnSparse *fDphiMixed, Float_t mult, Double_t zVtx){

    Double_t dphi_point[4];
    AliEventPool* fPool;
    fPool = fHHPoolMgr->GetEventPool(mult, zVtx); // Get the buffer associated with the current multiplicity and z-vtx
    if (!fPool)
    {
        AliFatal(Form("No pool found for centrality = %f, zVtx = %f", mult, zVtx));
        return;
    }
    
    Int_t nMix = fPool->GetCurrentNEvents();
    Int_t nTracks = 0;
    if(nMix >=5){
        for(int iMix=0; iMix < nMix; iMix++){            
            TObjArray *tracks = fPool->GetEvent(iMix);
            Long64_t numTracks = (Long64_t)tracks->GetEntriesFast();
            nTracks+= numTracks;
            for(Long64_t ihadron = 0; ihadron < tracks->GetEntriesFast(); ihadron++){
                AliCFParticle *hadron = (AliCFParticle*)tracks->At(ihadron);
                if(!hadron){
                    AliWarning(Form("ERROR: Could not receive mix pool track %d\n",ihadron));
                    continue;
                }
                dphi_point[0] = hadron->Pt();
                dphi_point[1] = assocPart->Pt();
                dphi_point[2] = hadron->Phi() - assocPart->Phi();
                if(dphi_point[2] < -TMath::Pi()/2.0){
                    dphi_point[2] += 2.0*TMath::Pi();
                }else if(dphi_point[2] > 3.0*TMath::Pi()/2.0){
                    dphi_point[2] -= 2.0*TMath::Pi();
                }
                dphi_point[3] = hadron->Eta() - assocPart->Eta();
                //dphi_point[4] = zVtx;
                Double_t weight = 1.0;
                if(fhEffHist !=0 && ftrigEffHist != 0){
                    weight = 1.0/(fhEffHist->GetBinContent(fhEffHist->GetXaxis()->FindBin(assocPart->Pt())));
                    weight = weight*(1.0/(ftrigEffHist->GetBinContent(ftrigEffHist->GetXaxis()->FindBin((hadron->Pt())))));
                }
                fDphiMixed->Fill(dphi_point, weight);
            }
        }
    }
    fHHMixStatZVtx->Fill(nMix, zVtx);
    fHHNoMixEvents->Fill(0);
    fHHMixTrackStatZVtx->Fill(nTracks, zVtx);
}    


//________________________________________________________________________
void AliAnalysisTaskHadronPhiCorr::UserExec(Option_t *){


    UInt_t evSelMask=((AliInputEventHandler*)(AliAnalysisManager::GetAnalysisManager()->GetInputEventHandler()))->IsEventSelected();
    
    fVevent = dynamic_cast<AliVEvent*>(InputEvent());
    if (!fVevent) {
        printf("ERROR: fVEvent not available\n");
        return;
    }
     
    fESD = dynamic_cast<AliESDEvent*>(InputEvent());
    ////////////////////
    //cuts initialised//
    ///////////////////
    AliESDtrackCuts* esdTrackCutsH = AliESDtrackCuts::GetStandardITSTPCTrackCuts2011(kFALSE);
    esdTrackCutsH->SetMaxDCAToVertexXY(2.4);
    esdTrackCutsH->SetMaxDCAToVertexZ(3.2);
    esdTrackCutsH->SetDCAToVertex2D(kTRUE);

    fAOD = dynamic_cast<AliAODEvent*>(InputEvent());
    if (fAOD) {
        // printf("fAOD available\n");
        //return;
    }

    ///////////////////
    //PID initialised//
    //////////////////
    fpidResponse = fInputHandler->GetPIDResponse();

    ////////////////
    //Event vertex//
    ///////////////
    Int_t ntracks = -999;
    ntracks = fVevent->GetNumberOfTracks();
    //if(ntracks < 1) printf("There are %d tracks in this event\n",ntracks);
    
    /////////////////
    //trigger check//
    /////////////////
    fVevent->GetFiredTriggerClasses();

    Int_t trigger = -1;
    //Multiplicity stuff
    Double_t multPercentile;
    if (fAOD){
        //Double_t multiplicity=fAOD->GetHeader()->GetRefMultiplicity();
        AliAODHeader *header = dynamic_cast<AliAODHeader*>(fAOD->GetHeader());
        if(!header) AliFatal("Not a standard AOD");
        Double_t multiplicity = header->GetRefMultiplicity();
 
        fMultSelection = (AliMultSelection*)fAOD->FindListObject("MultSelection");
        if(fMultSelection){
            multPercentile = fMultSelection->GetMultiplicityPercentile(CENT_ESTIMATOR.Data());
        }else{
            //return;
            multPercentile = 99.0;
        }
        fTrigMulti->Fill(-0.5, multiplicity);
        if(evSelMask & AliVEvent::kAny) fTrigMulti->Fill(0.5, multiplicity);
        if(evSelMask & AliVEvent::kMB) fTrigMulti->Fill(1.5, multiplicity);
        if(evSelMask & AliVEvent::kINT7) fTrigMulti->Fill(2.5, multiplicity);
        if(evSelMask & AliVEvent::kINT8) fTrigMulti->Fill(3.5, multiplicity);
        if(evSelMask & AliVEvent::kEMC1) fTrigMulti->Fill(4.5, multiplicity);
        if(evSelMask & AliVEvent::kEMC7) fTrigMulti->Fill(5.5, multiplicity);
        if(evSelMask & AliVEvent::kEMC8) fTrigMulti->Fill(6.5, multiplicity);
        if(evSelMask & AliVEvent::kEMCEJE) fTrigMulti->Fill(7.5, multiplicity);
        if(evSelMask & AliVEvent::kEMCEGA) fTrigMulti->Fill(8.5, multiplicity);

        if(multPercentile < MULT_LOW || multPercentile > MULT_HIGH) return;
    }
    
    fNevents->Fill(0); //all events
    Double_t Zvertex = -100, Xvertex = -100, Yvertex = -100;
    const AliVVertex *pVtx = fVevent->GetPrimaryVertex();
    Double_t NcontV = pVtx->GetNContributors();
    if(NcontV<3)return;
    fNevents->Fill(1); //events with 3 tracks

    Zvertex = pVtx->GetZ();
    Yvertex = pVtx->GetY();
    Xvertex = pVtx->GetX();
    fVtxZ->Fill(Zvertex);
    fVtxX->Fill(Xvertex);
    fVtxY->Fill(Yvertex);

    fNumTracks->Fill(ntracks);

    ////////////////////
    //event selection//
    ///////////////////
    if(Zvertex > Z_VTX_MAX || Zvertex < Z_VTX_MIN)return;

    fVtxZmixbins->Fill(Zvertex);

    Int_t indexZVtx = (Int_t)TMath::Floor((Zvertex - Z_VTX_MIN)*double(Z_VTX_NBINS)/(Z_VTX_MAX - Z_VTX_MIN));

    fNevents->Fill(2); //events after z vtx cut

    //Initialize the vectors/points that will be used to fill the histograms
    std::vector<AliPhiContainer> phiCandidates;
    std::vector<AliPhiContainer> truePhi;
    std::vector<AliPhiContainer> truePhiAcceptance;
    std::vector<AliPhiContainer> phiLikeSignCandidates;
    std::vector<AliHContainer> kPlusList;
    std::vector<AliHContainer> kMinusList;
    std::vector<AliHContainer> trigList;

    Double_t distPoint[5] = {0, 0, 0, 0, 0};
    Double_t trigPoint[3] = {0, 0, 0};
    Double_t dphi_point[5] = {0, 0, 0, 0, 0};
    Double_t hhdphi_point[4] = {0, 0, 0, 0};

    AliVTrack *kaonTrack = 0x0;
    AliESDtrack *eKaonTrack = 0x0;
    AliAODTrack *aKaonTrack = 0x0;
    AliVParticle *vKaonTrack = 0x0;

    Int_t numTriggers=0;
    Int_t numHadrons=0;
    Int_t numPhi=0;

    Double_t highestTrigPT = 0.0;
    Int_t highestTrigIndex = 0;

    //if MC events, do loop through MCArray to get generated particles
    if((IS_MC_TRUE || IS_MC_KAON)){
        TClonesArray* MCArray = dynamic_cast<TClonesArray*>(fAOD->FindListObject(AliAODMCParticle::StdBranchName()));
        if(!MCArray){
            AliError("Array of MC particles not found");
            return;
        }
        if(SELECT_TRIGGER_EVENTS){ //do initial loop to check if there are trigger particle in the event
            for(Int_t imcpart=0; imcpart< MCArray->GetEntries(); imcpart++){
                AliAODMCParticle *AODMCtrack = (AliAODMCParticle*)MCArray->At(imcpart);
                Int_t pdgcode = AODMCtrack->GetPdgCode();
                //select generated particles by their pdg codes for MC TRUE case
                if(IS_MC_TRUE){
                    if((TMath::Abs(pdgcode)==211 || TMath::Abs(pdgcode)==2212 || TMath::Abs(pdgcode)==11 || TMath::Abs(pdgcode)==321 || TMath::Abs(pdgcode)==13)){
                        if(AODMCtrack->Pt() >= 4.0 && AODMCtrack->Pt() <=8.0 && TMath::Abs(AODMCtrack->Eta()) <=0.8){
                            numTriggers++; //count number of triggers in event
                        }
                    }
                }
            }
            if(DO_SINGLE_TRIGGER){
                if(numTriggers !=1) return;
            }
        }  
        for(Int_t imcpart=0; imcpart< MCArray->GetEntries(); imcpart++){
            AliAODMCParticle *AODMCtrack = (AliAODMCParticle*)MCArray->At(imcpart);
            Int_t pdgcode = AODMCtrack->GetPdgCode();
            //select generated particles by their pdg codes for MC TRUE case
            if(IS_MC_TRUE){
                if((TMath::Abs(pdgcode)==211 || TMath::Abs(pdgcode)==2212 || TMath::Abs(pdgcode)==11 || TMath::Abs(pdgcode)==321 || TMath::Abs(pdgcode)==13)){
                    //if(AODMCtrack->Pt() >= 4.0 && AODMCtrack->Pt() <=8.0) numTriggers++; //count number of triggers in event
                    Double_t hdistpoint[] = {AODMCtrack->Pt(), AODMCtrack->Phi(), AODMCtrack->Eta(), AODMCtrack->Y(), 0};
                    switch(TMath::Abs(pdgcode)){
                        case 211: hdistpoint[4] = 1;
                                  break;
                        case 321: hdistpoint[4] = 2;
                                  break;
                        case 2212: hdistpoint[4] = 3;
                                  break;
                        case 11: hdistpoint[4] = 4;
                                 break;
                        case 13: hdistpoint[4] = 5;
                                 break;
                        default: break;
                    }
                    fTrueHDist->Fill(hdistpoint);
                    if(AODMCtrack->IsPhysicalPrimary()){
                        fTruePrimHDist->Fill(hdistpoint);
                    }
                    if(AODMCtrack->IsSecondaryFromWeakDecay()){
                        fTrueSecHDist->Fill(hdistpoint);
                    }
                    if(!AODMCtrack->IsPhysicalPrimary()){
                        fTrueNotPrimHDist->Fill(hdistpoint);
                    }
                    //if at least 1 trigger particle, fill single particle distribitions of the triggered distributions
                    if(numTriggers > 0 && SELECT_TRIGGER_EVENTS){
                        fTriggeredTrueHDist->Fill(hdistpoint);
                        if(AODMCtrack->IsPhysicalPrimary()){
                            fTriggeredTruePrimHDist->Fill(hdistpoint);
                            if(AODMCtrack->Pt() > 2.0 && AODMCtrack->Pt()< 4.0 && TMath::Abs(AODMCtrack->Eta())< 0.8){
                                numHadrons++; //count number of hadrons in acceptance in triggered events
                            }
                        }
                        if(AODMCtrack->IsSecondaryFromWeakDecay()){
                            fTriggeredTrueSecHDist->Fill(hdistpoint);
                        }
                        if(!AODMCtrack->IsPhysicalPrimary()){
                            fTriggeredTrueNotPrimHDist->Fill(hdistpoint);
                        }
                        
                    }
                }
                if(TMath::Abs(pdgcode) != 333) continue;
                Int_t indexFirstDaughter = 0, indexSecondDaughter = 0;
                indexFirstDaughter = AODMCtrack->GetDaughterFirst();
                indexSecondDaughter = AODMCtrack->GetDaughterLast();

                if(indexFirstDaughter < 0 || indexSecondDaughter < 0) continue;
                AliAODMCParticle* firstDaughter = (AliAODMCParticle*)MCArray->At(indexFirstDaughter);
                AliAODMCParticle* secondDaughter = (AliAODMCParticle*)MCArray->At(indexSecondDaughter);

                //select only phi that decay to two kaons
                if(TMath::Abs(firstDaughter->GetPdgCode()) == 321 && TMath::Abs(secondDaughter->GetPdgCode()) == 321 && (firstDaughter->GetPdgCode())*(secondDaughter->GetPdgCode()) <0){
                    AliPhiContainer phi;
                    phi.particle.SetPx(AODMCtrack->Px());
                    phi.particle.SetPy(AODMCtrack->Py());
                    phi.particle.SetPz(AODMCtrack->Pz());
                    phi.particle.SetE(AODMCtrack->E());
                    phi.daughter1TrackNum = indexFirstDaughter;
                    phi.daughter2TrackNum = indexSecondDaughter;
                    truePhi.push_back(phi);
                    Double_t shifted_phi = phi.particle.Phi();
                    if(shifted_phi < 0) shifted_phi += 2.0*TMath::Pi();
                    Double_t distpoint[] = {phi.particle.Pt(), phi.particle.M(), shifted_phi, phi.particle.Eta(), phi.particle.Rapidity()};
                    fTruePhiDist->Fill(distpoint);
                    if(numTriggers > 0 && SELECT_TRIGGER_EVENTS){
                        fTriggeredTruePhiDist->Fill(distpoint);
                        if(phi.particle.Pt() > 2.0 && phi.particle.Pt() < 4.0 && TMath::Abs(phi.particle.Eta())< 0.8){
                            numPhi++; //count number of phi in acceptance in triggered events
                        }
                    }
                    if(TMath::Abs(firstDaughter->Eta()) <= KAON_ETA_CUT && TMath::Abs(secondDaughter->Eta()) <= KAON_ETA_CUT && TMath::Abs(phi.particle.Eta()) < KAON_ETA_CUT){
                        truePhiAcceptance.push_back(phi);
                    }
                }
            // in MC Kaon case, select generated kaons and pair them combinatorially
            }else{
                if(TMath::Abs(pdgcode) != 321) continue;
                AliHContainer kaon;
                kaon.trackNum = imcpart;
                kaon.particle.SetPx(AODMCtrack->Px());
                kaon.particle.SetPy(AODMCtrack->Py());
                kaon.particle.SetPz(AODMCtrack->Pz());
                Double_t calcP = TMath::Sqrt(AODMCtrack->Px()*AODMCtrack->Px() + AODMCtrack->Py()*AODMCtrack->Py() + AODMCtrack->Pz()*AODMCtrack->Pz());
                Double_t calcE = TMath::Sqrt(0.4937*0.4937 + calcP*calcP);
                kaon.particle.SetE(calcE);

                if(pdgcode == 321){
                    if(!USE_ACCPT || TMath::Abs(AODMCtrack->Eta()) < 0.8){
                        kPlusList.push_back(kaon);
                    }
                }else if(pdgcode == -321){
                    if(!USE_ACCPT || TMath::Abs(AODMCtrack->Eta()) < 0.8){
                        kMinusList.push_back(kaon);
                    }
                }
                //fKaonPID->Fill(kaon.particle.Pt(), fTPCnSigma, fTOFnSigma);
                fKaonDist->Fill(kaon.particle.Pt(), kaon.particle.Phi(), kaon.particle.Eta());

            }
        }

        fHadronsVsPhi->Fill(numHadrons, numPhi);
        fHadronsVsPhiPairs->Fill(numTriggers*numHadrons, numTriggers*numPhi);
        if(numTriggers > 0 && numHadrons > 0){
            fRatio->Fill(Double_t(numPhi)/Double_t(numHadrons));
            fPairRatio->Fill(Double_t(numPhi*numTriggers)/Double_t(numHadrons*numTriggers));
        }
    }else{
        /* First Loop - Filling two vector for all Kaons (plus and minus) and filing vector of triggers*/
        for(Int_t itrack = 0; itrack < ntracks; itrack++){
            vKaonTrack = 0x0;
            vKaonTrack = fVevent->GetTrack(itrack);

            if(!vKaonTrack){
                printf("Error: Could not receive track %d\n", itrack);
                continue;
            }
            kaonTrack = dynamic_cast<AliVTrack*>(vKaonTrack);
            eKaonTrack = dynamic_cast<AliESDtrack*>(vKaonTrack);
            aKaonTrack = dynamic_cast<AliAODTrack*>(vKaonTrack);

            //make list of trigger particles
            if(fAOD && !IS_MC_TRUE){
                if(aKaonTrack->TestBit(TRIG_TRK_BIT) && TMath::Abs(kaonTrack->Eta()) < 0.8 && kaonTrack->Pt() > 4.0 && kaonTrack->Pt() < 8.0){
                    numTriggers++;
                    AliHContainer trig;
                    trig.trackNum = itrack;
                    trig.particle.SetPx(kaonTrack->Px());
                    trig.particle.SetPy(kaonTrack->Py());
                    trig.particle.SetPz(kaonTrack->Pz());
                    trigList.push_back(trig);
                    if(trig.particle.Pt() > highestTrigPT){
                        highestTrigPT = trig.particle.Pt();
                        highestTrigIndex = trigList.size()-1;
                    }
                }
            }

            if(!aKaonTrack->TestFilterMask(KAON_TRK_BIT)) continue; //mimimum cuts

            if(fESD){
                if(!esdTrackCutsH->AcceptTrack(eKaonTrack)) continue;
            }

            // Cut on pT and eta for possible Kaons
            if(kaonTrack->Pt() > 0.15 && TMath::Abs(kaonTrack->Eta()) < KAON_ETA_CUT){
                Double_t fTPCnSigma = -999;
                Double_t fTOFnSigma = -999;
                Double_t fpiTPCnSigma = -999;
                //check for labels
                Int_t label = 0;
                label = kaonTrack->GetLabel();

                fTPCnSigma = fpidResponse->NumberOfSigmasTPC(kaonTrack, AliPID::kKaon);
                fTOFnSigma = fpidResponse->NumberOfSigmasTOF(kaonTrack, AliPID::kKaon);
                //Cut on kaon candidates
                Bool_t acceptKaon = kFALSE;
                if(IS_MC_KTRACK){
                    if(label>0){
                        TClonesArray* MCArray = dynamic_cast<TClonesArray*>(fAOD->FindListObject(AliAODMCParticle::StdBranchName()));
                        if(!MCArray){
                            AliError("Array of MC particles not found");
                            return;
                        }
                        AliAODMCParticle *AODMCtrack = (AliAODMCParticle*)MCArray->At(label);
                        Int_t pdgcode = AODMCtrack->GetPdgCode();
                        //only accept tracks that have TOF hit that correspond to real Kaons
                        if(TMath::Abs(pdgcode) == 321 && fTOFnSigma != -999){
                            acceptKaon = kTRUE;
                        }
                    }
                }else{
                    if(IS_KAON_TOF_VETO){
                        acceptKaon = ((TMath::Abs(fTPCnSigma) <= KAON_TPC_CUT) && (((TMath::Abs(fTOFnSigma) <= KAON_TOF_CUT)) || ( fTOFnSigma == -999)));
                    }else{
                        acceptKaon = ((TMath::Abs(fTPCnSigma) <= KAON_TPC_CUT) && (TMath::Abs(fTOFnSigma) <= KAON_TOF_CUT));
                        //change min number of TPC crossed rows required
                        //acceptKaon = (acceptKaon && (aKaonTrack->GetTPCCrossedRows() > 80));
                    }
                }
                if(acceptKaon){
                    AliHContainer kaon;
                    kaon.trackNum = itrack;
                    kaon.particle.SetPx(kaonTrack->Px());
                    kaon.particle.SetPy(kaonTrack->Py());
                    kaon.particle.SetPz(kaonTrack->Pz());
                    Double_t calcP = TMath::Sqrt(kaonTrack->Px()*kaonTrack->Px() + kaonTrack->Py()*kaonTrack->Py() + kaonTrack->Pz()*kaonTrack->Pz());
                    Double_t calcE = TMath::Sqrt(0.4937*0.4937 + calcP*calcP);
                    kaon.particle.SetE(calcE);

                    if(kaonTrack->Charge() == 1){
                        kPlusList.push_back(kaon);
                    }else{
                        kMinusList.push_back(kaon);
                    }
                    fKaonPID->Fill(kaon.particle.Pt(), fTPCnSigma, fTOFnSigma);
                    fKaonDist->Fill(kaon.particle.Pt(), kaon.particle.Phi(), kaon.particle.Eta());
                }
            }
        }
    }
    
    fTrigPerEvent->Fill(numTriggers);

    if(DO_SINGLE_TRIGGER && numTriggers != 1) return;

    //if there aren't enough kaons to make pairs in this event, return
    //if((kPlusList.size() + kMinusList.size()) < 2) return;


    // Go through the Kaon lists and create the phi candidates and like sign pairs
    // Also fill in the US and LS K pair distribution histograms
    if(!IS_MC_TRUE && !IS_HH){
        AliPhiContainer phi;
        for(Int_t i_kplus = 0; i_kplus < (int)kPlusList.size(); i_kplus++){
            for(Int_t j_kplus = i_kplus+1; j_kplus < (int)kPlusList.size(); j_kplus++){
                phi.particle.SetPx(kPlusList[i_kplus].particle.Px() + kPlusList[j_kplus].particle.Px());
                phi.particle.SetPy(kPlusList[i_kplus].particle.Py() + kPlusList[j_kplus].particle.Py());
                phi.particle.SetPz(kPlusList[i_kplus].particle.Pz() + kPlusList[j_kplus].particle.Pz());
                phi.particle.SetE(kPlusList[i_kplus].particle.E() + kPlusList[j_kplus].particle.E());
                phi.daughter1TrackNum = kPlusList[i_kplus].trackNum;
                phi.daughter2TrackNum = kPlusList[j_kplus].trackNum;

                distPoint[0] = phi.particle.Pt();
                distPoint[1] = phi.particle.M();
                distPoint[2] = phi.particle.Phi();
                if(distPoint[2] < 0){
                    distPoint[2] += 2.0*TMath::Pi(); //change from range (-Pi, Pi) to (0, 2Pi)
                }
                distPoint[3] = phi.particle.Eta();
                distPoint[4] = phi.particle.Rapidity();

                //accept only those kaon pairs that fall within our mass range:
                if(phi.particle.M() > 1.07 || phi.particle.M()<0.98) continue;
                //accept only phi candidates that fall in our momentum range:
                if(phi.particle.Pt() < 0.5 || phi.particle.Pt() > 10) continue;
                //cut out all reconstructed phi at wide eta
                if(TMath::Abs(phi.particle.Eta()) >0.8) continue;

                //check for eta-phi range set for efficiency crosscheck
                if(ETA_PHI_REGION <= 0){
                    phiLikeSignCandidates.push_back(phi);
                    fKKLSDist->Fill(distPoint);
                }else if(ETA_PHI_REGION == 1 && TMath::Abs(phi.particle.Eta()) > 0.2 && distPoint[2] < TMath::Pi()){
                    phiLikeSignCandidates.push_back(phi);
                    fKKLSDist->Fill(distPoint);
                }else if(ETA_PHI_REGION == 2 && TMath::Abs(phi.particle.Eta()) <= 0.2 && distPoint[2] < TMath::Pi()){
                    phiLikeSignCandidates.push_back(phi);
                    fKKLSDist->Fill(distPoint);
                }else if(ETA_PHI_REGION == 3 && TMath::Abs(phi.particle.Eta()) > 0.2 && distPoint[2] >= TMath::Pi()){
                    phiLikeSignCandidates.push_back(phi);
                    fKKLSDist->Fill(distPoint);
                }else if(ETA_PHI_REGION == 4 && TMath::Abs(phi.particle.Eta()) <= 0.2 && distPoint[2] >= TMath::Pi()){
                    phiLikeSignCandidates.push_back(phi);
                    fKKLSDist->Fill(distPoint);
                }
            }
            for(Int_t i_kminus =0; i_kminus < (int)kMinusList.size(); i_kminus++){
                phi.particle.SetPx(kPlusList[i_kplus].particle.Px() + kMinusList[i_kminus].particle.Px());
                phi.particle.SetPy(kPlusList[i_kplus].particle.Py() + kMinusList[i_kminus].particle.Py());
                phi.particle.SetPz(kPlusList[i_kplus].particle.Pz() + kMinusList[i_kminus].particle.Pz());
                phi.particle.SetE(kPlusList[i_kplus].particle.E() + kMinusList[i_kminus].particle.E());
                phi.daughter1TrackNum = kPlusList[i_kplus].trackNum;
                phi.daughter2TrackNum = kMinusList[i_kminus].trackNum;

                distPoint[0] = phi.particle.Pt();
                distPoint[1] = phi.particle.M();
                distPoint[2] = phi.particle.Phi();
                if(distPoint[2] < 0){
                    distPoint[2] += 2.0*TMath::Pi();
                }
                distPoint[3] = phi.particle.Eta();
                distPoint[4] = phi.particle.Rapidity();

                //cut out all reconstructed phi at wide eta
                if(TMath::Abs(phi.particle.Eta()) >0.8) continue;

                //accept only those kaon pairs that fall within our mass range:
                if(phi.particle.M() < 1.07 && phi.particle.M() > 0.98){
                    //check for eta-phi range set for efficiency crosscheck
                    if(ETA_PHI_REGION <= 0){
                        phiCandidates.push_back(phi);
                        Double_t weight = 1.0/(fphiEffHist->GetBinContent(fphiEffHist->GetXaxis()->FindBin(phi.particle.Pt())));
                        fKKUSDist->Fill(distPoint, weight);
                    }else if(ETA_PHI_REGION == 1 && TMath::Abs(phi.particle.Eta()) > 0.2 && distPoint[2] < TMath::Pi()){
                        phiCandidates.push_back(phi);
                        fKKUSDist->Fill(distPoint);
                    }else if(ETA_PHI_REGION == 2 && TMath::Abs(phi.particle.Eta()) <= 0.2 && distPoint[2] < TMath::Pi()){
                        phiCandidates.push_back(phi);
                        fKKUSDist->Fill(distPoint);
                    }else if(ETA_PHI_REGION == 3 && TMath::Abs(phi.particle.Eta()) > 0.2 && distPoint[2] >= TMath::Pi()){
                        phiCandidates.push_back(phi);
                        fKKUSDist->Fill(distPoint);
                    }else if(ETA_PHI_REGION == 4 && TMath::Abs(phi.particle.Eta()) <= 0.2 && distPoint[2] >= TMath::Pi()){
                        phiCandidates.push_back(phi);
                        fKKUSDist->Fill(distPoint);
                    }
                }
            }
        }
        for(Int_t i_kminus =0; i_kminus < (int)kMinusList.size(); i_kminus++){
            for(Int_t j_kminus = i_kminus+1; j_kminus < (int)kMinusList.size(); j_kminus++){
                phi.particle.SetPx(kMinusList[i_kminus].particle.Px() + kMinusList[j_kminus].particle.Px());
                phi.particle.SetPy(kMinusList[i_kminus].particle.Py() + kMinusList[j_kminus].particle.Py());
                phi.particle.SetPz(kMinusList[i_kminus].particle.Pz() + kMinusList[j_kminus].particle.Pz());
                phi.particle.SetE(kMinusList[i_kminus].particle.E() + kMinusList[j_kminus].particle.E());
                phi.daughter1TrackNum = kMinusList[i_kminus].trackNum;
                phi.daughter2TrackNum = kMinusList[j_kminus].trackNum;


                distPoint[0] = phi.particle.Pt();
                distPoint[1] = phi.particle.M();
                distPoint[2] = phi.particle.Phi();
                if(distPoint[2] < 0){
                    distPoint[2] += 2.0*TMath::Pi();
                }
                distPoint[3] = phi.particle.Eta();
                distPoint[4] = phi.particle.Rapidity();

                //cut out all reconstructed phi at wide eta
                if(TMath::Abs(phi.particle.Eta()) >0.8) continue;

                //accept only those kaon pairs that fall within our mass range for our phi list:
                if(phi.particle.M() < 1.07 && phi.particle.M() > 0.98){
                    //check for eta-phi range set for efficiency crosscheck
                    if(ETA_PHI_REGION <= 0){
                        phiLikeSignCandidates.push_back(phi);
                        fKKLSDist->Fill(distPoint);
                    }else if(ETA_PHI_REGION == 1 && TMath::Abs(phi.particle.Eta()) > 0.2 && distPoint[2] < TMath::Pi()){
                        phiLikeSignCandidates.push_back(phi);
                        fKKLSDist->Fill(distPoint);
                    }else if(ETA_PHI_REGION == 2 && TMath::Abs(phi.particle.Eta()) <= 0.2 && distPoint[2] < TMath::Pi()){
                        phiLikeSignCandidates.push_back(phi);
                        fKKLSDist->Fill(distPoint);
                    }else if(ETA_PHI_REGION == 3 && TMath::Abs(phi.particle.Eta()) > 0.2 && distPoint[2] >= TMath::Pi()){
                        phiLikeSignCandidates.push_back(phi);
                        fKKLSDist->Fill(distPoint);
                    }else if(ETA_PHI_REGION == 4 && TMath::Abs(phi.particle.Eta()) <= 0.2 && distPoint[2] >= TMath::Pi()){
                        phiLikeSignCandidates.push_back(phi);
                        fKKLSDist->Fill(distPoint);
                    }

                }
            }
        }


        // Record how many kaons and kaon pairs are in the event
        fkplusPerEvent->Fill(kPlusList.size());
        fkminusPerEvent->Fill(kMinusList.size());
        fLSpairsPerEvent->Fill(phiLikeSignCandidates.size());
        fUSpairsPerEvent->Fill(phiCandidates.size());

    }

    ///////////////////////////////
    // Building d-phi histograms //
    ///////////////////////////////
    TObjArray* fArrayTracksMix = new TObjArray;
    fArrayTracksMix->SetOwner(kTRUE);

    TObjArray* fArrayLSTracksMix = new TObjArray;
    fArrayLSTracksMix->SetOwner(kTRUE);

    TObjArray* fArrayHHTracksMix = new TObjArray;
    fArrayHHTracksMix->SetOwner(kTRUE);

    TObjArray* fArrayTrueTracksMix = new TObjArray;
    fArrayTrueTracksMix->SetOwner(kTRUE);

    AliVTrack *triggerTrack = 0x0;
    AliESDtrack *etriggerTrack = 0x0;
    AliAODTrack *atriggerTrack = 0x0;
    AliVParticle* VtriggerTrack = 0x0;
    AliCFParticle *cfPart = 0x0;
    AliCFParticle *hhAssoc = new AliCFParticle(0.0, 0.0, 0.0, 0, 0);

    /* Do Correlations for MC case using the MCArray information */
    if(IS_MC_TRUE){
        TClonesArray* MCArray = dynamic_cast<TClonesArray*>(fAOD->FindListObject(AliAODMCParticle::StdBranchName()));
        if(!MCArray){
            AliError("Array of MC particles not found");
            return;
        }
        for(Int_t imcpart=0; imcpart< MCArray->GetEntries(); imcpart++){
            AliAODMCParticle *AODMCtrig = (AliAODMCParticle*)MCArray->At(imcpart);
            Int_t triggerpdgcode = TMath::Abs(AODMCtrig->GetPdgCode());
            if((TMath::Abs(triggerpdgcode)==211 || TMath::Abs(triggerpdgcode)==2212 || TMath::Abs(triggerpdgcode)==11 || TMath::Abs(triggerpdgcode)==321 || TMath::Abs(triggerpdgcode)==13) && TMath::Abs(AODMCtrig->Eta()) < 0.8 && AODMCtrig->Pt() > 2.0 /*&& AODMCtrig->IsPhysicalPrimary()*/){
                trigPoint[0] = AODMCtrig->Pt();
                trigPoint[1] = AODMCtrig->Phi();
                trigPoint[2] = AODMCtrig->Eta();
                fTrigDist->Fill(trigPoint);

                if(IS_HH){
                    for(Int_t iassoc = 0; iassoc< MCArray->GetEntries(); iassoc++){
                        if(iassoc == imcpart) continue;
                        AliAODMCParticle* AODMCassoc = (AliAODMCParticle*)MCArray->At(iassoc);
                        Int_t assocpdgcode = TMath::Abs(AODMCassoc->GetPdgCode());
                        if((TMath::Abs(assocpdgcode)==211 || TMath::Abs(assocpdgcode)==2212 || TMath::Abs(assocpdgcode)==11 || TMath::Abs(assocpdgcode)==321 || TMath::Abs(assocpdgcode)==13) && TMath::Abs(AODMCassoc->Eta()) < 0.8 && AODMCassoc->Pt() > 1.0 && AODMCassoc->IsPhysicalPrimary()){ //select charged hadrons
                        //if(TMath::Abs(assocpdgcode)==2212 && TMath::Abs(AODMCassoc->Eta()) < 0.8 && AODMCassoc->Pt() > 1.0 && AODMCassoc->IsPhysicalPrimary()) //select only protons
                        //if(TMath::Abs(assocpdgcode)==211 && TMath::Abs(AODMCassoc->Eta()) < 0.8 && AODMCassoc->Pt() > 1.0 && AODMCassoc->IsPhysicalPrimary()) //select only pions
                        //if(TMath::Abs(assocpdgcode)==321 && TMath::Abs(AODMCassoc->Eta()) < 0.8 && AODMCassoc->Pt() > 1.0 && AODMCassoc->IsPhysicalPrimary()) //select only kaons
                            hhdphi_point[0] = trigPoint[0];
                            hhdphi_point[1] = AODMCassoc->Pt();
                            hhdphi_point[2] = trigPoint[1] - AODMCassoc->Phi();
                            if(hhdphi_point[2] < -TMath::Pi()/2.0){
                                hhdphi_point[2] += 2.0*TMath::Pi();
                            }else if(hhdphi_point[2] > 3.0*TMath::Pi()/2.0){
                                hhdphi_point[2] -= 2.0*TMath::Pi();
                            }
                            hhdphi_point[3] = trigPoint[2] - AODMCassoc->Eta();
                            fDphiHH[indexZVtx]->Fill(hhdphi_point);
                            cfPart = new AliCFParticle(AODMCtrig->Pt(), AODMCtrig->Eta(), AODMCtrig->Phi(), AODMCtrig->Charge(), 0);
                            fTrigHHDist->Fill(AODMCtrig->Pt(), Zvertex);
                            hhAssoc->SetPt(AODMCassoc->Pt());
                            hhAssoc->SetEta(AODMCassoc->Eta());
                            hhAssoc->SetPhi(AODMCassoc->Phi());
                            hhAssoc->SetCharge(AODMCassoc->Charge());
                            if(fHHPoolMgr->GetEventPool(multPercentile, Zvertex)->IsReady()){
                                MakeHHMixCorrelations(hhAssoc, fDphiHHMixed[indexZVtx], multPercentile, Zvertex);
                            }
                            fArrayHHTracksMix->Add(cfPart);
                        }
                    }
                }else{
                    Bool_t isTrueDaughter = MakeCorrelations(imcpart, AODMCtrig, truePhi, fDphiTrueHPhi[indexZVtx], Zvertex);
                    Bool_t isTrueAcceptanceDaughter = MakeCorrelations(imcpart, AODMCtrig, truePhiAcceptance, fDphiTrueAcceptanceHPhi[indexZVtx], Zvertex);
                    if(!isTrueAcceptanceDaughter){
                        cfPart = new AliCFParticle(AODMCtrig->Pt(), AODMCtrig->Eta(), AODMCtrig->Phi(), AODMCtrig->Charge(), 0);
                        fArrayTrueTracksMix->Add(cfPart);
                    }
                }
            }
        }
    }else{
        for(int itrack = 0; itrack< trigList.size(); itrack++){

            if(DO_HIGHEST_TRIGGER && itrack != highestTrigIndex) continue;

            AliHContainer triggerTrack = trigList[itrack];

            trigPoint[0] = triggerTrack.particle.Pt();
            trigPoint[1] = triggerTrack.particle.Phi();
            if(trigPoint[1] < 0){
                trigPoint[1] += 2.0*TMath::Pi();
            }
            trigPoint[2] = triggerTrack.particle.Eta();
            Float_t weight = 1.0;
            if(ftrigEffHist->GetBinContent(ftrigEffHist->GetXaxis()->FindBin((triggerTrack.particle.Pt()))) == 0){
                AliFatal(Form("Trigger Efficiency Evaluated to 0 for pT %f", triggerTrack.particle.Pt()));
            }else{
                weight = 1.0/ftrigEffHist->GetBinContent(ftrigEffHist->GetXaxis()->FindBin((triggerTrack.particle.Pt())));
            }
            
            fTrigDist->Fill(trigPoint, weight);

            //hadron-phi correlations
            if(!IS_HH){

                Bool_t isTriggerDaughter = kTRUE;
                Bool_t isTriggerLSDaughter = kTRUE;
                Bool_t isTriggerAccptDaughter = kTRUE;
                Bool_t isTriggerTrueDaughter = kTRUE;
                
                isTriggerDaughter = MakeCorrelations(itrack, triggerTrack, phiCandidates, fDphiHPhi[indexZVtx], Zvertex, trigList.size());
                isTriggerLSDaughter = MakeCorrelations(itrack, triggerTrack, phiLikeSignCandidates, fDphiHKK[indexZVtx], Zvertex, trigList.size());

                if(!isTriggerDaughter){
                    cfPart = new AliCFParticle(triggerTrack.particle.Pt(), triggerTrack.particle.Eta(), trigPoint[1] ,triggerTrack.charge, 0);
                    fTrigSameUSDist->Fill(triggerTrack.particle.Pt(), Zvertex); //filled once per trigger, only if the trigger isn't a US pair daughter
                    fArrayTracksMix->Add(cfPart);
                }
                if(!isTriggerLSDaughter){
                    cfPart = new AliCFParticle(triggerTrack.particle.Pt(), triggerTrack.particle.Eta(), trigPoint[1], triggerTrack.charge, 0);
                    fTrigSameLSDist->Fill(triggerTrack.particle.Pt(), Zvertex); //filled once per trigger, only if the trigger isn't a LS pair daughter
                    fArrayLSTracksMix->Add(cfPart);
                }
            }
            //di-hadron correlations
            if(IS_HH){
                for(Int_t jtrack = 0; jtrack < ntracks; jtrack++){
                    if(triggerTrack.trackNum != jtrack){
                        AliVTrack *assocTrack = 0x0;
                        AliESDtrack *eassocTrack = 0x0;
                        AliAODTrack *aassocTrack = 0x0;
                        AliVParticle* VassocTrack = 0x0;
                        VassocTrack = fVevent->GetTrack(jtrack);

                        aassocTrack = dynamic_cast<AliAODTrack*>(VassocTrack);

                        if(aassocTrack->TestFilterMask(ASSOC_TRK_BIT) && aassocTrack->Pt() > 0.5 && aassocTrack->Pt() < 10 && TMath::Abs(aassocTrack->Eta())<0.8){
                            hhdphi_point[0] = trigPoint[0];
                            hhdphi_point[1] = aassocTrack->Pt();
                            hhdphi_point[2] = trigPoint[1] - aassocTrack->Phi();
                            if(hhdphi_point[2] < -TMath::Pi()/2.0){
                                hhdphi_point[2] += 2.0*TMath::Pi();
                            }else if(hhdphi_point[2] > 3.0*TMath::Pi()/2.0){
                                hhdphi_point[2] -= 2.0*TMath::Pi();
                            }
                            hhdphi_point[3] = trigPoint[2] - aassocTrack->Eta();
                            //hhdphi_point[4] = Zvertex;
                            //hhdphi_point[4] = multPercentile;
                            Double_t weight = 1.0;
                            if(fhEffHist !=0 && ftrigEffHist !=0){
                                weight = 1.0/fhEffHist->GetBinContent(fhEffHist->GetXaxis()->FindBin((aassocTrack->Pt())));
                                weight = weight*(1.0/ftrigEffHist->GetBinContent(ftrigEffHist->GetXaxis()->FindBin((trigPoint[0]))));
                            }
                            if(DO_PER_EVENT_SCALING){
                                weight = weight/trigList.size();
                            }
                            fDphiHH[indexZVtx]->Fill(hhdphi_point, weight);
                            hhAssoc->SetPt(aassocTrack->Pt());
                            hhAssoc->SetEta(aassocTrack->Eta());
                            hhAssoc->SetPhi(aassocTrack->Phi());
                            hhAssoc->SetCharge(aassocTrack->Charge());
                            if(fHHPoolMgr->GetEventPool(multPercentile, Zvertex)->IsReady()){
                                MakeHHMixCorrelations(hhAssoc, fDphiHHMixed[indexZVtx], multPercentile, Zvertex);
                            }
                        }
                    }
                }
                if(multPercentile <= 100.0){
                    cfPart = new AliCFParticle(triggerTrack.particle.Pt(), triggerTrack.particle.Eta(), trigPoint[1], triggerTrack.charge, 0);
                    fTrigHHDist->Fill(triggerTrack.particle.Pt(), Zvertex);
                    fArrayHHTracksMix->Add(cfPart);
                }
            }
    }   //track loop
}
delete hhAssoc;


//if there was a trigger, fill additional inclusive histograms
if(numTriggers>0){
    if(phiCandidates.size() >0 ){
        for(int iphi = 0; iphi < phiCandidates.size(); iphi++){
            distPoint[0] = phiCandidates[iphi].particle.Pt();
            distPoint[1] = phiCandidates[iphi].particle.M();
            distPoint[2] = phiCandidates[iphi].particle.Phi();
            if(distPoint[2] < 0){
                distPoint[2] += 2.0*TMath::Pi();
            }
            distPoint[3] = phiCandidates[iphi].particle.Eta();
            distPoint[4] = phiCandidates[iphi].particle.Rapidity();
            Double_t weight = 1.0/fphiEffHist->GetBinContent(fphiEffHist->GetXaxis()->FindBin((phiCandidates[iphi].particle.Pt())));
            fKKUSTrigDist->Fill(distPoint, weight);
        }
    }

    if(phiLikeSignCandidates.size() >0){
        for(int iphi = 0; iphi < phiLikeSignCandidates.size(); iphi++){
            distPoint[0] = phiLikeSignCandidates[iphi].particle.Pt();
            distPoint[1] = phiLikeSignCandidates[iphi].particle.M();
            distPoint[2] = phiLikeSignCandidates[iphi].particle.Phi();
            if(distPoint[2] < 0){
                distPoint[2] += 2.0*TMath::Pi();
            }
            distPoint[3] = phiLikeSignCandidates[iphi].particle.Eta();
            distPoint[4] = phiLikeSignCandidates[iphi].particle.Rapidity();
            fKKLSTrigDist->Fill(distPoint);
        }
    }


}

ntracks = fVevent->GetNumberOfTracks();

if(multPercentile <= 100.){
    if(!IS_HH){
        if(IS_MC_TRUE){
            if(truePhiAcceptance.size() > 0){
                AliEventPool *fTruePool = 0x0;
                fTruePool = fTruePoolMgr->GetEventPool(multPercentile, Zvertex);
                if(!fTruePool){
                    AliFatal(Form("No true pool found for multiplicity = %f, zVtx = %i", multPercentile, Zvertex));
                    return;
                }else{
                    if(fTruePool->IsReady()){
                        for(int i = 0; i < truePhiAcceptance.size(); i++){
                            MakeMixCorrelations(&truePhiAcceptance[i], fDphiTrueHPhiMixed[indexZVtx], multPercentile, Zvertex, fTruePool, kFALSE);
                        }
                    }
                    if(fArrayTrueTracksMix->GetEntries() > 0){
                        fTruePool->UpdatePool(fArrayTrueTracksMix);
                    }
                }
            }
        }else{
            if(phiCandidates.size() > 0){
                AliEventPool *fPool = 0x0;
                fPool = fPoolMgr->GetEventPool(multPercentile, Zvertex); // Get the buffer associated with the current centrality and z-vtx
                if(!fPool){
                    AliFatal(Form("No pool found for multiplicity = %f, zVtx = %f", multPercentile, Zvertex));
                    return;
                }else{
                    if(fPool->IsReady()){
                        for(int i =0; i< phiCandidates.size(); i++){
                            MakeMixCorrelations(&phiCandidates[i], fDphiHPhiMixed[indexZVtx], multPercentile, Zvertex, fPool, kFALSE);
                        }
                    }
                    if(fArrayTracksMix->GetEntries() > 0){
                        fPool->UpdatePool(fArrayTracksMix);
                    }
                }
            }
        }
        if(phiLikeSignCandidates.size() > 0){
            AliEventPool *fLSPool = 0x0;
            fLSPool = fLSPoolMgr->GetEventPool(multPercentile, Zvertex); // Get the buffer associated with the current centrality and z-vtx
            if(!fLSPool){
                AliFatal(Form("No pool found for multiplicity = %f, zVtx = %f", multPercentile, Zvertex));
                return;
            }else{
                if(fLSPool->IsReady()){
                    for(int i =0; i< phiLikeSignCandidates.size(); i++){
                        MakeMixCorrelations(&phiLikeSignCandidates[i], fDphiHKKMixed[indexZVtx], multPercentile, Zvertex, fLSPool, kTRUE);
                    }
                }
                if(fArrayLSTracksMix->GetEntries() > 0){
                    fLSPool->UpdatePool(fArrayLSTracksMix);
                }
            }
        }

    }
    //di-hadron event pool
    if(IS_HH){
        AliEventPool *fHHPool = 0x0;
        fHHPool = fHHPoolMgr->GetEventPool(multPercentile, Zvertex);
        if(!fHHPool){
            AliFatal(Form("No pool found for multiplicity = %f, zVtx = %f", multPercentile, Zvertex));
            return;
        }else if(fArrayHHTracksMix->GetEntries() > 0){
            fHHPool->UpdatePool(fArrayHHTracksMix);
        }
    }
}
PostData(1, fOutputList);
}
//________________________________________________________________________
void AliAnalysisTaskHadronPhiCorr::Terminate(Option_t *) 
{
    // Draw result to the screen
    // Called once at the end of the query
    printf("terminating task... \n");
    fOutputList = dynamic_cast<TList*> (GetOutputData(1));
    if (!fOutputList) {
        printf("ERROR: Output list not available\n");
        return;
    }    
}
