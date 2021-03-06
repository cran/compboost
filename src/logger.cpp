// ========================================================================== //
//                                 ___.                          __           //
//        ____  ____   _____ ______\_ |__   ____   ____  _______/  |_         //
//      _/ ___\/  _ \ /     \\____ \| __ \ /  _ \ /  _ \/  ___/\   __\        //
//      \  \__(  <_> )  Y Y  \  |_> > \_\ (  <_> |  <_> )___ \  |  |          //
//       \___  >____/|__|_|  /   __/|___  /\____/ \____/____  > |__|          //
//           \/            \/|__|       \/                  \/                //
//                                                                            //
// ========================================================================== //
//
// Compboost is free software: you can redistribute it and/or modify
// it under the terms of the MIT License.
// Compboost is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// MIT License for more details. You should have received a copy of 
// the MIT License along with compboost. 
//
// Written by:
// -----------
//
//   Daniel Schalk
//   Department of Statistics
//   Ludwig-Maximilians-University Munich
//   Ludwigstrasse 33
//   D-80539 München
//
//   https://www.compstat.statistik.uni-muenchen.de
//
//   Contact
//   e: contact@danielschalk.com
//   w: danielschalk.com
//
// =========================================================================== #

#include "logger.h"

namespace logger
{

// -------------------------------------------------------------------------- //
// Abstract 'Logger' class:
// -------------------------------------------------------------------------- //

/**
 * \brief Getter if the logger is used as stopper
 * 
 * \returns `bool` which says `true` if it is a logger, otherwise `false`
 */
bool Logger::getIfLoggerIsStopper () const
{
  return is_a_stopper;
}

// Destructor:
Logger::~Logger () { }





// -------------------------------------------------------------------------- //
// Logger implementations:
// -------------------------------------------------------------------------- //

// LoggerIteration:
// -----------------------

/**
 * \brief Default constructor of class `LoggerIteration`
 * 
 * Sets the private member `max_iteration` and the tag if the logger should be
 * used as stopper.
 * 
 * \param is_a_stopper `bool` specify if the logger should be used as stopper
 * \param max_iterations `unsigned int` sets value of the stopping criteria
 * 
 */

LoggerIteration::LoggerIteration (const bool& is_a_stopper0, 
  const unsigned int& max_iterations) 
  : max_iterations ( max_iterations ) 
{
  is_a_stopper = is_a_stopper0;
}

/**
 * \brief Log current step of compboost iteration of class `LoggerIteration`
 * 
 * This function loggs the current iteration. 
 * 
 * \param current_iteration `unsigned int` of current iteration 
 * \param response `arma::vec` of the given response used for training
 * \param prediction `arma::vec` actual prediction of the boosting model at 
 *   iteration `current_iteration`
 * \param used_blearner `Baselearner*` pointer to the selected baselearner in 
 *   iteration `current_iteration`
 * \param offset `double` of the overall offset of the training
 * \param learning_rate `double` lerning rate of the `current_iteration`
 * 
 */

void LoggerIteration::logStep (const unsigned int& current_iteration, const arma::vec& response, 
  const arma::vec& prediction, blearner::Baselearner* used_blearner, const double& offset, 
  const double& learning_rate)
{
  iterations.push_back(current_iteration);
}

/**
 * \brief Stop criteria is fulfilled if the current iteration exceed `max_iteration`
 * 
 * 
 * 
 * \returns `bool` which tells if the stopping criteria is reached or not 
 *   (if the logger isn't a stopper then this is always false)
 */

bool LoggerIteration::reachedStopCriteria () const
{
  bool stop_criteria_is_reached = false;
  
  if (is_a_stopper) {
    if (max_iterations <= iterations.back()) {
      stop_criteria_is_reached = true;
    }
  }
  return stop_criteria_is_reached;
}

/**
 * \brief Return the data stored within the iteration logger
 * 
 * This function returns the logged integer. An issue here is, that the later
 * transformation of all logged data to an `arma::mat` requires `arma::vec` as
 * return value. Therefore the std integer vector is transforemd to an 
 * `arma::vec`. We know that this isn't very memory friendly, but the 
 * `arma::mat` we use later can just have one type.
 * 
 * \return `arma::vec` of iterations.
 */

arma::vec LoggerIteration::getLoggedData () const
{
  // Cast integer vector to double:
  std::vector<double> iterations_double (iterations.begin(), iterations.end());
  
  arma::vec out (iterations_double);
  return out;
}

/**
 * \brief Clear the logger data
 * 
 * This is an important thing which is called every time in front of retraining 
 * the model. If we don't clear the data, the new iterations are just pasted at
 * the end of the existing vectors which couses some troubles. 
 */

void LoggerIteration::clearLoggerData ()
{
  iterations.clear();
}

/**
 * \brief Print status of current iteration into the console 
 * 
 * The string which is created in this functions must have exactly the same
 * length as the string from `initializeLoggerPrinter()`. Those strings are
 * printed line by line.
 * 
 * \returns `std::string` which includes the log of the current iteration
 */

std::string LoggerIteration::printLoggerStatus () const
{
  std::string max_iters = std::to_string(max_iterations);
  std::stringstream ss;
  ss << std::setw(2 * max_iters.size() + 1) << std::to_string(iterations.back()) + "/" + max_iters;
  
  return ss.str();
}





// InbagRisk:
// -----------------------

/**
 * \brief Default constructor of class `LoggerInbagRisk`
 * 
 * \param is_a_stopper0 `bool` specify if the logger should be used as stopper
 * \param used_loss `Loss*` used loss to calculate the empirical risk (this 
 *   can differ from the one used while training the model)
 * \param eps_for_break `double` sets value of the stopping criteria`
 */

LoggerInbagRisk::LoggerInbagRisk (const bool& is_a_stopper0, loss::Loss* used_loss, 
  const double& eps_for_break)
  : used_loss ( used_loss ),
    eps_for_break ( eps_for_break )
{
  is_a_stopper = is_a_stopper0;
}

/**
 * \brief Log current step of compboost iteration for class `LoggerInbagRisk`
 * 
 * This logger computes the risk for the given training data
 * \f$\mathcal{D}_\mathrm{train} = \{(x_i,\ y_i)\ |\ i \in \{1, \dots, n\}\}\f$
 * and stores it into a vector. The empirical risk \f$\mathcal{R}\f$ for 
 * iteration \f$m\f$ is calculated by:
 * \f[
 *   \mathcal{R}_\mathrm{emp}^{[m]} = \frac{1}{|\mathcal{D}_\mathrm{train}|}\sum\limits_{(x,y) \in \mathcal{D}_\mathrm{train}} L(y, \hat{f}^{[m]}(x))
 * \f]
 * 
 * **Note:** 
 *   - If \f$m=0\f$ than \f$\hat{f}\f$ is just the offset.
 *   - The implementation to calculate \f$\mathcal{R}_\mathrm{emp}^{[m]}\f$ is
 *     done in two steps: 
 *        1. Calculate vector `risk_temp` of losses for every observation for 
 *           given response \f$y^{(i)}\f$ and prediction \f$\hat{f}^{[m]}(x^{(i)})\f$.
 *        2. Average over `risk_temp`.
 *         
 *    This procedure ensures, that it is possible to e.g. use the AUC or any
 *    arbitrary performance measure for risk logging. This gives just one 
 *    value for `risk_temp` and therefore the average equals the loss 
 *    function. If this is just a value (like for the AUC) then the value is 
 *    returned.
 * 
 * \param current_iteration `unsigned int` of current iteration 
 * \param response `arma::vec` of the given response used for training
 * \param prediction `arma::vec` actual prediction of the boosting model at 
 *   iteration `current_iteration`
 * \param used_blearner `Baselearner*` pointer to the selected baselearner in 
 *   iteration `current_iteration`
 * \param offset `double` of the overall offset of the training
 * \param learning_rate `double` lerning rate of the `current_iteration`
 * 
 */

void LoggerInbagRisk::logStep (const unsigned int& current_iteration, const arma::vec& response, 
  const arma::vec& prediction, blearner::Baselearner* used_blearner, const double& offset, 
  const double& learning_rate)
{
  // Calculate empirical risk. Calculateion of the temporary vector ensures
  // // that stuff like auc logging is possible:
  // arma::vec loss_vec_temp = used_loss->definedLoss(response, prediction);
  // double temp_risk = arma::accu(loss_vec_temp) / loss_vec_temp.size();

  double temp_risk = arma::mean(used_loss->definedLoss(response, prediction));
  
  tracked_inbag_risk.push_back(temp_risk);
}

/**
 * \brief Stop criteria is fulfilled if the relative improvement falls below `eps_for_break`
 * 
 * The stopping criteria is fulfilled, if the relative improvement at the 
 * current iteration \f$m\f$ \f$\varepsilon^{[m]}\f$ falls under a fixed boundary 
 * \f$\varepsilon\f$. Where the relative improvement is defined by
 * \f[
 *   \varepsilon^{[m]} = \frac{\mathcal{R}_\mathrm{emp}^{[m-1]} - \mathcal{R}_\mathrm{emp}^{[m]}}{\mathcal{R}_\mathrm{emp}^{[m-1]}}.
 * \f]
 * 
 * The logger stops the algorithm if \f$\varepsilon^{[m]} \leq \varepsilon\f$.
 * 
 * \returns `bool` which tells if the stopping criteria is reached or not 
 *   (if the logger isn't a stopper then this is always false)
 */

bool LoggerInbagRisk::reachedStopCriteria () const
{
  bool stop_criteria_is_reached = false;
  
  if (is_a_stopper) {
    if (tracked_inbag_risk.size() > 1) {
      // We need to subtract -2 and -1 since c++ start counting by 0 while size
      // returns the actual number of elements, so if just one element exists
      // size returns 1 but we want to access 0:
      double inbag_eps = tracked_inbag_risk[tracked_inbag_risk.size() - 2] - tracked_inbag_risk[tracked_inbag_risk.size() - 1];
      inbag_eps = inbag_eps / tracked_inbag_risk[tracked_inbag_risk.size() - 2];
      
      if (inbag_eps <= eps_for_break) {
        stop_criteria_is_reached = true;
      }
    }
  }
  return stop_criteria_is_reached;
}

/**
 * \brief Return the data stored within the OOB risk logger
 * 
 * This function returns the logged OOB risk.
 * 
 * \return `arma::vec` of elapsed time
 */

arma::vec LoggerInbagRisk::getLoggedData () const
{
  arma::vec out (tracked_inbag_risk);
  return out;
}

/**
 * \brief Clear the logger data
 * 
 * This is an important thing which is called every time in front of retraining 
 * the model. If we don't clear the data, the new iterations are just pasted at
 * the end of the existing vectors which couses some troubles. 
 */

void LoggerInbagRisk::clearLoggerData ()
{
  tracked_inbag_risk.clear();
}

/**
 * \brief Print status of current iteration into the console 
 * 
 * The string which is created in this functions must have exactly the same
 * length as the string from `initializeLoggerPrinter()`. Those strings are
 * printed line by line.
 * 
 * \returns `std::string` which includes the log of the current iteration
 */

std::string LoggerInbagRisk::printLoggerStatus () const
{
  std::stringstream ss;
  ss << std::setw(17) << std::fixed << std::setprecision(2) << tracked_inbag_risk.back();
  
  return ss.str();
}





// OobRisk:
// -----------------------


/**
 * \brief Default constructor of `LoggerOobRisk`
 * 
 * \param is_a_stopper0 `bool` to set if the logger should be used as stopper
 * \param used_loss `Loss*` which is used to calculate the empirical risk (this 
 *   can differ from the loss used while trining the model)
 * \param eps_for_break `double` sets value of the stopping criteria
 * \param oob_data `std::map<std::string, data::Data*>` the new data
 * \param oob_response `arma::vec` response of the new data
 */

LoggerOobRisk::LoggerOobRisk (const bool& is_a_stopper0, loss::Loss* used_loss, 
  const double& eps_for_break, std::map<std::string, data::Data*> oob_data, 
  const arma::vec& oob_response)
  : used_loss ( used_loss ),
    eps_for_break ( eps_for_break ),
    oob_data ( oob_data ),
    oob_response ( oob_response )
{
  is_a_stopper = is_a_stopper0;
  
  arma::vec temp (oob_response.size());
  oob_prediction = temp;
}

/**
 * \brief Log current step of compboost iteration for class `LoggerOobRisk`
 * 
 * This logger computes the risk for a given new dataset 
 * \f$\mathcal{D}_\mathrm{oob} = \{(x_i,\ y_i)\ |\ i \in I_\mathrm{oob}\}\f$
 * and stores it into a vector. The OOB risk \f$\mathcal{R}_\mathrm{oob}\f$ for 
 * iteration \f$m\f$ is calculated by:
 * \f[
 *   \mathcal{R}_\mathrm{oob}^{[m]} = \frac{1}{|\mathcal{D}_\mathrm{oob}|}\sum\limits_{(x,y) \in \mathcal{D}_\mathrm{oob}} 
 *   L(y, \hat{f}^{[m]}(x))
 * \f]
 * 
 * **Note:** 
 *   - If \f$m=0\f$ than \f$\hat{f}\f$ is just the offset.
 *   - The implementation to calculate \f$\mathcal{R}_\mathrm{oob}^{[m]}\f$ is
 *     done in two steps: 
 *        1. Calculate vector `risk_temp` of losses for every observation for 
 *           given response \f$y^{(i)}\f$ and prediction \f$\hat{f}^{[m]}(x^{(i)})\f$.
 *        2. Average over `risk_temp`.
 *         
 *    This procedure ensures, that it is possible to e.g. use the AUC or any
 *    arbitrary performance measure for risk logging. This gives just one 
 *    value for `risk_temp` and therefore the average equals the loss 
 *    function. If this is just a value (like for the AUC) then the value is 
 *    returned.
 * 
 * \param current_iteration `unsigned int` of current iteration 
 * \param response `arma::vec` of the given response used for training
 * \param prediction `arma::vec` actual prediction of the boosting model at 
 *   iteration `current_iteration`
 * \param used_blearner `Baselearner*` pointer to the selected baselearner in 
 *   iteration `current_iteration`
 * \param offset `double` of the overall offset of the training
 * \param learning_rate `double` lerning rate of the `current_iteration`
 * 
 */

void LoggerOobRisk::logStep (const unsigned int& current_iteration, const arma::vec& response, 
  const arma::vec& prediction, blearner::Baselearner* used_blearner, const double& offset, 
  const double& learning_rate)
{
  if (current_iteration == 1) {
    oob_prediction.fill(offset);
  }
  
  // Get data of corresponding selected baselearner. E.g. iteration 100 linear 
  // baselearner of feature x_7, then get the data of feature x_7:
  data::Data* oob_blearner_data = oob_data.find(used_blearner->getDataIdentifier())->second;
  
  // Predict this data using the selected baselearner:
  arma::vec temp_oob_prediction = used_blearner->predict(oob_blearner_data);
  
  // Cumulate prediction and shrink by learning rate:
  oob_prediction += learning_rate * temp_oob_prediction;
  
  // Calculate empirical risk. Calculateion of the temporary vector ensures
  // that stuff like auc logging is possible:
  arma::mat loss_vec_temp = used_loss->definedLoss(oob_response, oob_prediction);
  double temp_risk = arma::accu(loss_vec_temp) / loss_vec_temp.size();
  
  // Track empirical risk:
  tracked_oob_risk.push_back(temp_risk);
}

/**
 * \brief Stop criteria is fulfilled if the relative improvement falls below 
 *   `eps_for_break`
 * 
 * The stopping criteria is fulfilled, if the relative improvement at the 
 * current iteration \f$m\f$ \f$\varepsilon^{[m]}\f$ falls under a fixed boundary 
 * \f$\varepsilon\f$. Where the relative improvement is defined by
 * \f[
 *   \varepsilon^{[m]} = \frac{\mathcal{R}_\mathrm{oob}^{[m-1]} - \mathcal{R}_\mathrm{oob}^{[m]}}{\mathcal{R}_\mathrm{oob}^{[m-1]}}.
 * \f]
 * 
 * The logger stops the algorithm if \f$\varepsilon^{[m]} \leq \varepsilon\f$
 * 
 * \returns `bool` which tells if the stopping criteria is reached or not 
 *   (if the logger isn't a stopper then this is always false)
 */

bool LoggerOobRisk::reachedStopCriteria () const
{
  bool stop_criteria_is_reached = false;
  
  if (is_a_stopper) {
    if (tracked_oob_risk.size() > 1) {
      // We need to subtract -2 and -1 since c++ start counting by 0 while size
      // returns the actual number of elements, so if just one element exists
      // size returns 1 but we want to access 0:
      double oob_eps = tracked_oob_risk[tracked_oob_risk.size() - 2] - tracked_oob_risk[tracked_oob_risk.size() - 1];
      oob_eps = oob_eps / tracked_oob_risk[tracked_oob_risk.size() - 2];
      
      if (oob_eps <= eps_for_break) {
        stop_criteria_is_reached = true;
      }
    }
  }
  return stop_criteria_is_reached;
}

/**
 * \brief Return the data stored within the OOB risk logger
 * 
 * This function returns the logged OOB risk.
 * 
 * \return `arma::vec` of elapsed out of bag risk
 */

arma::vec LoggerOobRisk::getLoggedData () const
{
  arma::vec out (tracked_oob_risk);
  return out;
}

/**
 * \brief Clear the logger data
 * 
 * This is an important thing which is called every time in front of retraining 
 * the model. If we don't clear the data, the new iterations are just pasted at
 * the end of the existing vectors which couses some troubles. 
 */
void LoggerOobRisk::clearLoggerData ()
{
  tracked_oob_risk.clear();
}

/**
 * \brief Print status of current iteration into the console 
 * 
 * The string which is created in this functions must have exactly the same
 * length as the string from `initializeLoggerPrinter()`. Those strings are
 * printed line by line.
 * 
 * \returns `std::string` which includes the log of the current iteration
 */

std::string LoggerOobRisk::printLoggerStatus () const
{
  std::stringstream ss;
  ss << std::setw(17) << std::fixed << std::setprecision(2) << tracked_oob_risk.back();
  
  return ss.str();
}





// LoggerTime:
// -----------------------

/**
 * \brief Default constructor of class `LoggerTime`
 * 
 * \param is_a_stopper0 `bool` which specifies if the logger is used as stopper
 * \param max_time `unsigned int` maximal time for training (just used if logger 
 *   is a stopper)
 * \param time_unit `std::string` of the unit used for measuring, allowed are 
 *   `minutes`, `seconds` and `microseconds`
 */

LoggerTime::LoggerTime (const bool& is_a_stopper0, const unsigned int& max_time, 
  const std::string& time_unit)
  : max_time ( max_time ),
    time_unit ( time_unit )
{
  // This is necessary to prevent the program from segfolds... whyever???
  // Copied from: http://lists.r-forge.r-project.org/pipermail/rcpp-devel/2012-November/004796.html
  try {
    if (time_unit != "minutes" ) {
      if (time_unit != "seconds") {
        if (time_unit != "microseconds") {
          Rcpp::stop("Time unit has to be one of 'microseconds', 'seconds' or 'minutes'.");
        }
      }
    }
  } catch ( std::exception &ex ) {
    forward_exception_to_r( ex );
  } catch (...) { 
    ::Rf_error( "c++ exception (unknown reason)" ); 
  }
  is_a_stopper = is_a_stopper0;
}

/**
 * \brief Log current step of compboost iteration for class `LoggerTime`
 * 
 * This functions loggs dependent on `time_unit` the elapsed time at the
 * current iteration.
 * 
 * \param current_iteration `unsigned int` of current iteration 
 * \param response `arma::vec` of the given response used for training
 * \param prediction `arma::vec` actual prediction of the boosting model at 
 *   iteration `current_iteration`
 * \param used_blearner `Baselearner*` pointer to the selected baselearner in 
 *   iteration `current_iteration`
 * \param offset `double` of the overall offset of the training
 * \param learning_rate `double` lerning rate of the `current_iteration`
 * 
 */

void LoggerTime::logStep (const unsigned int& current_iteration, const arma::vec& response, 
  const arma::vec& prediction, blearner::Baselearner* used_blearner, const double& offset, 
  const double& learning_rate)
{
  if (current_time.size() == 0) {
    init_time = std::chrono::steady_clock::now();
  }
  if (time_unit == "minutes") {
    current_time.push_back(std::chrono::duration_cast<std::chrono::minutes>(std::chrono::steady_clock::now() - init_time).count());
  } 
  if (time_unit == "seconds") {
    current_time.push_back(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - init_time).count());
  } 
  if (time_unit == "microseconds") {
    current_time.push_back(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - init_time).count());
  }
}

/**
 * \brief Stop criteria is fulfilled if the passed time exceeds `max_time`
 * 
 * The stop criteria here is quite simple. For the current iteration \f$m\f$ it 
 * is triggered if 
 * \f[
 *   \mathrm{current_time}_m > \mathrm{max_time}
 * \f]
 * 
 * \returns `bool` which tells if the stopping criteria is reached or not 
 *   (if the logger isn't a stopper then this is always false)
 */

bool LoggerTime::reachedStopCriteria () const
{
  bool stop_criteria_is_reached = false;
  
  if (is_a_stopper) {
    if (current_time.back() >= max_time) {
      stop_criteria_is_reached = true;
    }
  }
  return stop_criteria_is_reached;
}

/**
 * \brief Return the data stored within the time logger
 * 
 * This function returns the logged elapsed time. An issue here is, that the 
 * later transformation of all logged data to an `arma::mat` requires 
 * `arma::vec` as return value. Therefore the std integer vector is transforemd 
 * to an `arma::vec`. We know that this isn't very memory friendly, but the 
 * `arma::mat` we use later can just have one type.
 * 
 * \return `arma::vec` of elapsed time
 */

arma::vec LoggerTime::getLoggedData () const
{
  // Cast integer vector to double:
  std::vector<double> seconds_double (current_time.begin(), current_time.end());
  
  arma::vec out (seconds_double);
  return out;
}

/**
 * \brief Clear the logger data
 * 
 * This is an important thing which is called every time in front of retraining 
 * the model. If we don't clear the data, the new iterations are just pasted at
 * the end of the existing vectors which couses some troubles. 
 */

void LoggerTime::clearLoggerData ()
{
  current_time.clear();
}

/**
 * \brief Print status of current iteration into the console 
 * 
 * The string which is created in this functions must have exactly the same
 * length as the string from `initializeLoggerPrinter()`. Those strings are
 * printed line by line.
 * 
 * \returns `std::string` which includes the log of the current iteration
 */

std::string LoggerTime::printLoggerStatus () const
{
  std::stringstream ss;
  ss << std::setw(17) << std::fixed << std::setprecision(2) << current_time.back();
  
  return ss.str();
}

} // namespace logger
