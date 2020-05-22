////////////////////////////////////////////////////////////////////////
// Class:       CalibrationdEdXPDSP
// Module Type: producer
// File:        CalibrationdEdXPDSP_module.cc
//
// Generated at Thu Nov 30 15:55:16 2017 by Tingjun Yang using artmod
// from cetpkgsupport v1_13_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "canvas/Persistency/Common/FindManyP.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "lardata/Utilities/AssociationUtil.h"
#include "lardata/DetectorInfoServices/DetectorPropertiesService.h"
#include "larreco/Calorimetry/CalorimetryAlg.h"
#include "lardataobj/RecoBase/Track.h"
#include "lardataobj/AnalysisBase/Calorimetry.h"
#include "dune/Calib/XYZCalib.h"
#include "dune/CalibServices/XYZCalibService.h"
#include "dune/Calib/LifetimeCalib.h" 
#include "dune/CalibServices/LifetimeCalibService.h" 

#include "larevt/SpaceCharge/SpaceCharge.h"
#include "larevt/SpaceChargeServices/SpaceChargeService.h"

#include "TH2F.h"
#include "TH1F.h"
#include "TFile.h"
#include "TTimeStamp.h" 

#include <memory>

namespace dune {
  class CalibrationdEdXPDSP;
}

class dune::CalibrationdEdXPDSP : public art::EDProducer {
public:
  explicit CalibrationdEdXPDSP(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  CalibrationdEdXPDSP(CalibrationdEdXPDSP const &) = delete;
  CalibrationdEdXPDSP(CalibrationdEdXPDSP &&) = delete;
  CalibrationdEdXPDSP & operator = (CalibrationdEdXPDSP const &) = delete;
  CalibrationdEdXPDSP & operator = (CalibrationdEdXPDSP &&) = delete;

  // Required functions.
  void produce(art::Event & e) override;

private:

  std::string fTrackModuleLabel;
  std::string fCalorimetryModuleLabel;

  calo::CalorimetryAlg caloAlg;
  
  double fModBoxA;
  double fModBoxB;
  
  bool fSCE;
  bool fApplyNormCorrection;
  bool fApplyXCorrection;
  bool fApplyYZCorrection;
  bool fApplyLifetimeCorrection;
  bool fUseLifetimeFromDatebase; // true: lifetime from database; false: lifetime from DetectorProperties

  double fLifetime; // [us]

  double vDrift;
  double xAnode;
  const detinfo::DetectorProperties* detprop;

};


dune::CalibrationdEdXPDSP::CalibrationdEdXPDSP(fhicl::ParameterSet const & p)
  : EDProducer(p)
  , fTrackModuleLabel      (p.get< std::string >("TrackModuleLabel"))
  , fCalorimetryModuleLabel(p.get< std::string >("CalorimetryModuleLabel"))
  , caloAlg                (p.get< fhicl::ParameterSet >("CaloAlg"))
  , fModBoxA               (p.get< double >("ModBoxA"))
  , fModBoxB               (p.get< double >("ModBoxB"))
  , fSCE                   (p.get< bool >("CorrectSCE"))
  , fApplyNormCorrection   (p.get< bool >("ApplyNormCorrection"))
  , fApplyXCorrection      (p.get< bool >("ApplyXCorrection"))
  , fApplyYZCorrection     (p.get< bool >("ApplyYZCorrection"))
  , fApplyLifetimeCorrection(p.get< bool >("ApplyLifetimeCorrection"))
  , fUseLifetimeFromDatebase(p.get< bool >("UseLifetimeFromDatebase"))
{
  detprop = art::ServiceHandle<detinfo::DetectorPropertiesService>()->provider();
  vDrift = detprop->DriftVelocity(); // [cm/us]
  xAnode = std::abs(detprop->ConvertTicksToX(detprop->TriggerOffset(),0,0,0));
  //std::cout<<detprop->TriggerOffset()<<" "<<xAnode<<std::endl;
  //create calorimetry product and its association with track
  produces< std::vector<anab::Calorimetry>              >();
  produces< art::Assns<recob::Track, anab::Calorimetry> >();

}

void dune::CalibrationdEdXPDSP::produce(art::Event & evt)
{

  art::ServiceHandle<calib::XYZCalibService> xyzcalibHandler;
  calib::XYZCalibService & xyzcalibService = *xyzcalibHandler;
  calib::XYZCalib *xyzcalib = xyzcalibService.provider();
  
  // Electron lifetime from database calibration service provider
  art::ServiceHandle<calib::LifetimeCalibService> lifetimecalibHandler;
  calib::LifetimeCalibService & lifetimecalibService = *lifetimecalibHandler; 
  calib::LifetimeCalib *lifetimecalib = lifetimecalibService.provider();

  if (fUseLifetimeFromDatebase) {
    fLifetime = lifetimecalib->GetLifetime()*1000.0; // [ms]*1000.0 -> [us]
    //std::cout << "use lifetime from database   " << fLifetime << std::endl;
  } 
  else {
    fLifetime = detprop->ElectronLifetime(); // [us] 
  }
  
  //std::cout << "fLifetime: " << fLifetime << std::endl;

  //Spacecharge services provider 
  auto const* sce = lar::providerFrom<spacecharge::SpaceChargeService>();
  
  //create anab::Calorimetry objects and make association with recob::Track
  std::unique_ptr< std::vector<anab::Calorimetry> > calorimetrycol(new std::vector<anab::Calorimetry>);
  std::unique_ptr< art::Assns<recob::Track, anab::Calorimetry> > assn(new art::Assns<recob::Track, anab::Calorimetry>);

  //get existing track/calorimetry objects
  art::Handle< std::vector<recob::Track> > trackListHandle;
  evt.getByLabel(fTrackModuleLabel,trackListHandle);

  std::vector<art::Ptr<recob::Track> > tracklist;
  art::fill_ptr_vector(tracklist, trackListHandle);

  art::FindManyP<anab::Calorimetry> fmcal(trackListHandle, evt, fCalorimetryModuleLabel);

  if (!fmcal.isValid()){
    throw art::Exception(art::errors::ProductNotFound)
      <<"Could not get assocated Calorimetry objects";
  }

  for (size_t trkIter = 0; trkIter < tracklist.size(); ++trkIter){   
    for (size_t i = 0; i<fmcal.at(trkIter).size(); ++i){
      auto & calo = fmcal.at(trkIter)[i];
      
      if (!(calo->dEdx()).size()){
        //empty calorimetry product, just copy it
        calorimetrycol->push_back(*calo);
        util::CreateAssn(*this, evt, *calorimetrycol, tracklist[trkIter], *assn);
      }
      else{
        //start calibrating dQdx

        //get original calorimetry information
        //double                Kin_En     = calo->KineticEnergy();
        std::vector<float>   vdEdx      = calo->dEdx();
        std::vector<float>   vdQdx      = calo->dQdx();
        std::vector<float>   vresRange  = calo->ResidualRange();
        std::vector<float>   deadwire   = calo->DeadWireResRC();
        float                Trk_Length = calo->Range();
        std::vector<float>   fpitch     = calo->TrkPitchVec();
        const auto&          vXYZ       = calo->XYZ();
        geo::PlaneID         planeID    = calo->PlaneID();

        //make sure the vectors are of the same size
        if (vdEdx.size()!=vXYZ.size()||
            vdQdx.size()!=vXYZ.size()||
            vresRange.size()!=vXYZ.size()||
            fpitch.size()!=vXYZ.size()){
          throw art::Exception(art::errors::Configuration)
      <<"Vector sizes mismatch for vdEdx, vdQdx, vresRange, fpitch, vXYZ";
        }

        //make sure the planeID is reasonable
        if (!planeID.isValid){
          throw art::Exception(art::errors::Configuration)
      <<"planeID is invalid";
        }
        if (planeID.Plane>2){
          throw art::Exception(art::errors::Configuration)
            <<"plane is invalid "<<planeID.Plane;
        }
	// update the kinetic energy
	double EkinNew = 0.;

        for (size_t j = 0; j<vdQdx.size(); ++j){
          double normcorrection = 1;
          if (fApplyNormCorrection){
            normcorrection = xyzcalib->GetNormCorr(planeID.Plane);
            if (!normcorrection) normcorrection = 1.;
          }
          double xcorrection = 1;
          if (fApplyXCorrection){
            xcorrection = xyzcalib->GetXCorr(planeID.Plane, vXYZ[j].X());
            if (!xcorrection) xcorrection = 1.;
          }
          double yzcorrection = 1;
          if (fApplyYZCorrection){
            yzcorrection = xyzcalib->GetYZCorr(planeID.Plane, vXYZ[j].X()>0, vXYZ[j].Y(), vXYZ[j].Z());
            if (!yzcorrection) yzcorrection = 1.;
          }
          if (fApplyLifetimeCorrection){
            xcorrection *= exp((xAnode-std::abs(vXYZ[j].X()))/(fLifetime*vDrift));
          }
          //std::cout<<"plane = "<<planeID.Plane<<" x = "<<vXYZ[j].X()<<" y = "<<vXYZ[j].Y()<<" z = "<<vXYZ[j].Z()<<" normcorrection = "<<normcorrection<<" xcorrection = "<<xcorrection<<" yzcorrection = "<<yzcorrection<<std::endl;

          vdQdx[j] = normcorrection*xcorrection*yzcorrection*vdQdx[j];
          
          
          //set time to be trgger time so we don't do lifetime correction
          //we will turn off lifetime correction in caloAlg, this is just to be double sure
          //vdEdx[j] = caloAlg.dEdx_AREA(vdQdx[j], detprop->TriggerOffset(), planeID.Plane, 0);
          
          
          //Calculate dE/dx uisng the new recombination constants
          double dQdx_e = caloAlg.ElectronsFromADCArea(vdQdx[j], planeID.Plane);
          double rho = detprop->Density();  			// LAr density in g/cm^3
          double Wion = 1000./util::kGeVToElectrons;    // 23.6 eV = 1e, Wion in MeV/e
          double E_field_nominal = detprop->Efield();   // Electric Field in the drift region in KV/cm
          
          //correct Efield for SCE
          geo::Vector_t E_field_offsets = {0., 0., 0.};
          
          if(sce->EnableCalEfieldSCE()&&fSCE) E_field_offsets = sce->GetCalEfieldOffsets(geo::Point_t{vXYZ[j].X(), vXYZ[j].Y(), vXYZ[j].Z()},planeID.TPC);
          
          TVector3 E_field_vector = {E_field_nominal*(1 + E_field_offsets.X()), E_field_nominal*E_field_offsets.Y(), E_field_nominal*E_field_offsets.Z()};
          double E_field = E_field_vector.Mag();
          
          //calculate recombination factors
          double Beta = fModBoxB / (rho * E_field);
          double Alpha = fModBoxA;
          //double old_vdEdx = vdEdx[j];
          vdEdx[j] = (exp(Beta * Wion * dQdx_e) - Alpha) / Beta;
          
         /*if (planeID.Plane==2){ 
         std::cout << sce->EnableCalEfieldSCE() << " " << fSCE << std::endl;
         std::cout << E_field << " " << E_field_nominal << std::endl;
         std::cout << vdQdx[j] << " " << dQdx_e << std::endl;
         std::cout << old_vdEdx << " " << vdEdx[j] << std::endl; 
         std::cout << rho << " " << Wion << " " << Beta << "\n" << std::endl; }
         */ 
	  //update kinetic energy calculation
	  if (j>=1) {
	    if ( (vresRange[j] < 0) || (vresRange[j-1] < 0) ) continue;
	    EkinNew += fabs(vresRange[j]-vresRange[j-1]) * vdEdx[j];
	  }
	  if (j==0){
	    if ( (vresRange[j] < 0) || (vresRange[j+1] < 0) ) continue;
	    EkinNew += fabs(vresRange[j]-vresRange[j+1]) * vdEdx[j];
	  }
	  
        }
        //save new calorimetry information 
        calorimetrycol->push_back(anab::Calorimetry(EkinNew,// Kin_En, // change by David C. to update kinetic energy calculation
                                                    vdEdx,
                                                    vdQdx,
                                                    vresRange,
                                                    deadwire,
                                                    Trk_Length,
                                                    fpitch,
                                                    vXYZ,
                                                    planeID));
        util::CreateAssn(*this, evt, *calorimetrycol, tracklist[trkIter], *assn);
      }//calorimetry object not empty
    }//loop over calorimetry objects
  }//loop over tracks

  evt.put(std::move(calorimetrycol));
  evt.put(std::move(assn));

  return;
}

DEFINE_ART_MODULE(dune::CalibrationdEdXPDSP)
