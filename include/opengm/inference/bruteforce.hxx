#pragma once
#ifndef OPENGM_BRUTEFORCE_HXX
#define OPENGM_BRUTEFORCE_HXX

#include "inference.hxx"
#include "movemaker.hxx"
#include "opengm/inference/visitors/visitor.hxx"

namespace opengm {

template<class GM> class Movemaker;

/// Brute force inference algorithm 
///
/// \ingroup inference
template<class GM, class ACC>
class Bruteforce : public Inference<GM, ACC>
{
public:
   typedef ACC AccumulationType;
   typedef GM GraphicalModelType;
   OPENGM_GM_TYPE_TYPEDEFS;
   typedef typename std::vector<LabelType>::const_iterator LabelIterator;
   typedef EmptyVisitor< Bruteforce<GM, ACC> > EmptyVisitorType;
   typedef VerboseVisitor< Bruteforce<GM, ACC> > VerboseVisitorType;
   typedef TimingVisitor< Bruteforce<GM, ACC> > TimingVisitorType;
   class Parameter {};

   Bruteforce(const GraphicalModelType&);
   Bruteforce(const GraphicalModelType&, const Parameter&);
   std::string name() const
      { return "Brute-Force"; }
   const GraphicalModelType& graphicalModel() const
      { return gm_; }
   InferenceTermination infer()
      {
         EmptyVisitorType visitor;
         return infer(visitor);
      }
   template<class VISITOR> InferenceTermination infer(VISITOR &);
   InferenceTermination arg(std::vector<LabelType>&, const size_t = 1) const;
   void reset();

private:
    const GraphicalModelType& gm_;
    opengm::Movemaker<GraphicalModelType> movemaker_;
    std::vector<LabelType> states_;
    ValueType energy_;
};

template<class GM, class AKK>
Bruteforce<GM, AKK>::Bruteforce
(
    const GraphicalModelType& gm
)
:  gm_(gm),
   movemaker_(Movemaker<GM>(gm)),
   states_(std::vector<typename GM::LabelType>(gm.numberOfVariables() )),
   energy_(typename Bruteforce<GM, AKK>::ValueType())
{
   AKK::neutral(energy_);
}

template<class GM, class AKK>
void
Bruteforce<GM, AKK>::reset()
{
   movemaker_.reset();
   std::fill(states_.begin(), states_.end(), 0);
   AKK::neutral(energy_);
}

template<class GM, class AKK>
Bruteforce<GM, AKK>::Bruteforce
(
   const GraphicalModelType& gm,
   const typename Bruteforce<GM, AKK>::Parameter& param
)
:  gm_(gm),
   movemaker_(Movemaker<GM>(gm)),
   states_(std::vector<typename GM::LabelType>(gm.numberOfVariables())),
   energy_(typename Bruteforce<GM, AKK>::ValueType())
{}

template<class GM, class AKK>
template<class VISITOR>
InferenceTermination
Bruteforce<GM, AKK>::infer
(
   VISITOR & visitor
)
{
   std::vector<LabelType> states(gm_.numberOfVariables());
   std::vector<IndexType> vi(gm_.numberOfVariables());
   for(size_t j=0; j<gm_.numberOfVariables(); ++j) {
       vi[j] = j;
   }
   energy_ = movemaker_.move(vi.begin(), vi.end(), states.begin());
   visitor.begin(*this, energy_, energy_);
   for(;;) {
      ValueType energy = movemaker_.move(vi.begin(), vi.end(), states.begin());
      if(AccumulationType::bop(energy , energy_)) {
         visitor(*this, energy, energy);
         energy_ = energy;
         states_ = states;
      }
      bool overflow = true;
      for(size_t j=0; j<gm_.numberOfVariables(); ++j) {
         if( size_t(states[j]+1) < size_t(gm_.numberOfLabels(j))) {
            ++states[j];
            for(size_t k=0; k<j; ++k) {
               states[k] = 0;
            }
            overflow = false;
            break;
         }
      }
      if(overflow) {
         break;
      }
   }
   visitor.end(*this, energy_, energy_);
   return NORMAL;
}

template<class GM, class AKK>
inline InferenceTermination
Bruteforce<GM, AKK>::arg
(
   std::vector<LabelType>& states,
   const size_t j
) const
{
   if(j == 1) {
      states = states_; // copy
      return NORMAL;
   }
   else {
      return UNKNOWN;
   }
}

} // namespace opengm

#endif // #ifndef OPENGM_BRUTEFORCE_HXX
