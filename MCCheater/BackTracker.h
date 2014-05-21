////////////////////////////////////////////////////////////////////////
/// \file  BackTracker.h
/// \brief back track the reconstruction to the simulation
///
/// \version $Id: Geometry.h,v 1.16 2009/11/03 22:53:20 brebel Exp $
/// \author  brebel@fnal.gov
////////////////////////////////////////////////////////////////////////
#ifndef CHEAT_BACKTRACKER_H
#define CHEAT_BACKTRACKER_H

#include <vector>
#include <stdint.h>

#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"

#include "SimulationBase/MCParticle.h"
#include "Simulation/ParticleList.h"
#include "Simulation/SimChannel.h"
#include "Simulation/EveIdCalculator.h"
#include "SimulationBase/MCTruth.h"
#include "Simulation/LArVoxelList.h"
#include "SimpleTypesAndConstants/geo_types.h"

#include "Utilities/TimeService.h"

namespace recob{
  class Hit;
  class SpacePoint;
}

///code to link reconstructed objects back to the MC truth information
namespace cheat{

  typedef struct{
    int trackID;      ///< Geant4 supplied trackID
    float energyFrac; ///< fraction of hit energy from the particle with this trackID
    float energy;     ///< energy from the particle with this trackID
  } TrackIDE;

  class BackTracker
  {

  public:

    BackTracker(fhicl::ParameterSet const& pset,
		art::ActivityRegistry&     reg);
    ~BackTracker();

    void reconfigure(fhicl::ParameterSet const& pset);

    // The Rebuild function rebuilds the various maps we need to answer backtracking queries.
    // It is called automatically before each event is processed. For jobs involving
    // Monte Carlo generation, this is too soon. So, we'll call rebuild after those data
    // products are put into the event in LArG4.  This is the least bad way of ensuring the
    // BackTracker works in jobs that combine MC production and reconstruction analysis based
    // on MC truth.  Don't copy this design pattern without talking to brebel@fnal.gov first
    void Rebuild(const art::Event& evt);

    // Get a reference to the ParticleList
    const sim::ParticleList&          ParticleList()  const { return fParticleList; }

    // Set the EveIdCalculator for the owned ParticleList
    void  SetEveIdCalculator(sim::EveIdCalculator *ec) { fParticleList.AdoptEveIdCalculator(ec); }

    // Return a pointer to the simb::MCParticle object corresponding to
    // the given TrackID
    const simb::MCParticle*              TrackIDToParticle(int const& id)       const;
    const simb::MCParticle*              TrackIDToMotherParticle(int const& id) const;

    const std::vector<sim::IDE>          TrackIDToSimIDE(int const& id)         const;

    // Get art::Ptr<> to simb::MCTruth and related information
    const art::Ptr<simb::MCTruth>&       TrackIDToMCTruth(int const& id)                        const;
    const art::Ptr<simb::MCTruth>&       ParticleToMCTruth(const simb::MCParticle* p)           const;
    std::vector<const simb::MCParticle*> MCTruthToParticles(art::Ptr<simb::MCTruth> const& mct) const;
    const std::vector< art::Ptr<simb::MCTruth> >& MCTruthVector() const { return fMCTruthList; }

    // this method will return the Geant4 track IDs of 
    // the particles contributing ionization electrons to the identified hit
    const std::vector<TrackIDE> HitToTrackID(art::Ptr<recob::Hit> const& hit,
					     double tpc_start_time=0);
    
    // method to return a subset of allhits that are matched to a list of TrackIDs
    const std::vector<std::vector<art::Ptr<recob::Hit>>> TrackIDsToHits(std::vector<art::Ptr<recob::Hit>> const& allhits,
									std::vector<int> const& tkIDs,
									double tpc_start_time=0);
    
    // method to return the EveIDs of particles contributing ionization
    // electrons to the identified hit
    const std::vector<TrackIDE> HitToEveID(art::Ptr<recob::Hit> const& hit,
					   double tpc_start_time = 0);
    
    // method to return sim::IDE objects associated with a given hit
    void                 HitToSimIDEs(art::Ptr<recob::Hit> const& hit,
				      std::vector<sim::IDE>&      ides,
				      double tpc_start_time=0);
    
    // method to return the XYZ position of the weighted average energy deposition for a given hit
    std::vector<double>  SimIDEsToXYZ(std::vector<sim::IDE> const& ides);
    
    // method to return the XYZ position of the weighted average energy deposition for a given hit
    std::vector<double>  HitToXYZ(art::Ptr<recob::Hit> const& hit,
				  double tpc_start_time=0);
				  
    
    // method to return the XYZ position of a space point (unweighted average XYZ of component hits).
    std::vector<double> SpacePointToXYZ(art::Ptr<recob::SpacePoint> const& spt,
					art::Event                  const& evt,
					std::string                 const& label,
					double tpc_start_time=0);

    // method to return the XYZ position of a space point (unweighted average XYZ of component hits).
    std::vector<double> SpacePointHitsToXYZ(art::PtrVector<recob::Hit> const& hits,
					    double tpc_start_time = 0);
    
    // method to return the fraction of hits in a collection that come from the specified Geant4 track ids 
    double              HitCollectionPurity(std::set<int>                              trackIDs, 
					    std::vector< art::Ptr<recob::Hit> > const& hits,
					    double tpc_start_time = 0);
    
    // method to return the fraction of all hits in an event from a specific set of Geant4 track IDs that are 
    // represented in a collection of hits
    double              HitCollectionEfficiency(std::set<int>                              trackIDs, 
						std::vector< art::Ptr<recob::Hit> > const& hits,
						std::vector< art::Ptr<recob::Hit> > const& allhits,
						geo::View_t                         const& view,
						double tpc_start_time = 0);

    // method to return the fraction of charge in a collection that come from the specified Geant4 track ids 
    double              HitChargeCollectionPurity(std::set<int>                              trackIDs, 
						  std::vector< art::Ptr<recob::Hit> > const& hits,
						  double tpc_start_time = 0);
    
    // method to return the fraction of all charge in an event from a specific set of Geant4 track IDs that are 
    // represented in a collection of hits
    double              HitChargeCollectionEfficiency(std::set<int>                              trackIDs, 
						      std::vector< art::Ptr<recob::Hit> > const& hits,
						      std::vector< art::Ptr<recob::Hit> > const& allhits,
						      geo::View_t                         const& view,
						      double tpc_start_time = 0);  
  
    // method to return all EveIDs corresponding to the current sim::ParticleList
    std::set<int>       GetSetOfEveIDs();

    // method to return all TrackIDs corresponding to the current sim::ParticleList
    std::set<int>       GetSetOfTrackIDs();

    // method to return all EveIDs corresponding to the given list of hits
    std::set<int>       GetSetOfEveIDs(std::vector< art::Ptr<recob::Hit> > const& hits,
				       double tpc_start_time = 0);

    // method to return all TrackIDs corresponding to the given list of hits
    std::set<int>       GetSetOfTrackIDs(std::vector< art::Ptr<recob::Hit> > const& hits,
					 double tpc_start_time = 0);

    const std::vector<const sim::SimChannel*>& SimChannels() const { return fSimChannels; } 

  private:

    void ChannelToTrackID(std::vector<TrackIDE>& trackIDEs,
			  uint32_t               channel,
			  const double hit_start_time,
			  const double hit_end_time,
			  double tpc_start_time = 0);

    const sim::SimChannel* FindSimChannel(uint32_t channel) const;

    sim::ParticleList                      fParticleList;          ///< ParticleList to map track ID to sim::Particle
    sim::LArVoxelList                      fVoxelList;             ///< List to map the position of energy depostions
                                                                   ///< in voxels to the particles depositing the 
                                                                   ///< energy
    std::set<int>                          fTrackIDs;              ///< G4 track ids for all particles in the event
    std::vector< art::Ptr<simb::MCTruth> > fMCTruthList;           ///< all the MCTruths for the event
    std::vector<const sim::SimChannel*>    fSimChannels;           ///< all the SimChannels for the event
    std::map<int, int>                     fTrackIDToMCTruthIndex; ///< map of track ids to MCTruthList entry
    std::string                            fG4ModuleLabel;         ///< label for geant4 module
    double                                 fMinHitEnergyFraction;  ///< minimum fraction of energy a track id has to 
                                                                   ///< contribute to a hit to be counted in
                                                                   ///< purity and efficiency calculations 
                                                                   ///< based on hit collections
    ::util::ElecClock                      fTPCClock;              ///< TPC electronics clock
  };
} // namespace

DECLARE_ART_SERVICE(cheat::BackTracker, LEGACY)
#endif // CHEAT_BACKTRACK_H
