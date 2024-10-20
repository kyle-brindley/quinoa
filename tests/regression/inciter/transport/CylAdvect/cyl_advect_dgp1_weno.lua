inciter = {

  title = "Advection of cylinder",

  nstep = 50,    -- Max number of time steps
  dt = 1.0e-3,   -- Time step size
  ttyi = 10,     -- TTY output interval
  scheme = "dgp1",
  limiter = "wenop1",
  cweight = 200.0,

  transport = {
    physics = "advection",
    problem = "cyl_advect",
    ncomp = 1
  },

  bc = {
    {
      extrapolate = {1},
      dirichlet = {2},
      outlet = {3}
    }
  },

  diagnostics = {
    interval = 10,
    format = "scientific",
    error = "l2"
  },

  field_output = {
    interval = 50,
    elemvar = {"analytic", "C1"},
    elemalias = {"", "c0_numerical"}
  }

}
