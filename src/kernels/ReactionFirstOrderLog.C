#include "ReactionFirstOrderLog.h"

// MOOSE includes
#include "MooseVariable.h"

registerMooseObject("CraneApp", ReactionFirstOrderLog);

template <>
InputParameters
validParams<ReactionFirstOrderLog>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("v", "The first variable that is reacting to create u.");
  params.addRequiredParam<std::string>("reaction", "The full reaction equation.");
  params.addRequiredParam<Real>("coefficient", "The stoichiometric coeffient.");
  params.addParam<bool>("_v_eq_u", false, "If v == u.");
  params.addParam<std::string>(
      "number",
      "",
      "The reaction number. Optional, just for material property naming purposes. If a single "
      "reaction has multiple different rate coefficients (frequently the case when multiple "
      "species are lumped together to simplify a reaction network), this will prevent the same "
      "material property from being declared multiple times.");
  return params;
}

ReactionFirstOrderLog::ReactionFirstOrderLog(const InputParameters & parameters)
  : Kernel(parameters),
    _v(coupledValue("v")),
    _v_id(coupled("v")),
    _reaction_coeff(getMaterialProperty<Real>("k" + getParam<std::string>("number") + "_" +
                                              getParam<std::string>("reaction"))),
    _stoichiometric_coeff(getParam<Real>("coefficient")),
    _v_eq_u(getParam<bool>("_v_eq_u"))
{
}

Real
ReactionFirstOrderLog::computeQpResidual()
{
  return -_test[_i][_qp] * _stoichiometric_coeff * _reaction_coeff[_qp] * std::exp(_v[_qp]);
}

Real
ReactionFirstOrderLog::computeQpJacobian()
{
  Real power;
  power = 0.0;

  if (_v_eq_u)
  {
    power += 1.0;
  }

  if (!_v_eq_u)
  {
    return 0.0;
  }
  else
  {
    return -_test[_i][_qp] * _stoichiometric_coeff * power * _reaction_coeff[_qp] *
           std::exp(_v[_qp]) * _phi[_j][_qp];
  }
}

Real
ReactionFirstOrderLog::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (!_v_eq_u && jvar == _v_id)
  {
    return -_test[_i][_qp] * _stoichiometric_coeff * _reaction_coeff[_qp] * std::exp(_v[_qp]) *
           _phi[_j][_qp];
  }
  else
  {
    return 0.0;
  }
}
