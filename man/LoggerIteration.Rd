% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/RcppExports.R
\name{LoggerIteration}
\alias{LoggerIteration}
\title{Logger class to log the current iteration}
\format{\code{\link{S4}} object.}
\description{
This class seems to be useless, but it gives more control about the algorithm
and doesn't violate the idea of object programming here. Additionally, it is
quite convenient to have this class instead of tracking the iteration at any
stage of the fitting within the compboost object as another vector.
}
\section{Usage}{

\preformatted{
LoggerIterationWrapper$new(use_as_stopper, max_iterations)
}
}

\section{Arguments}{

\describe{
\item{\code{use_as_stopper} [\code{logical(1)}]}{
  Boolean to indicate if the logger should also be used as stopper.
}
\item{\code{max_iterations} [\code{integer(1)}]}{
  If the logger is used as stopper this argument defines the maximal
  iterations.
}
}
}

\section{Details}{


  This class is a wrapper around the pure \code{C++} implementation. To see
  the functionality of the \code{C++} class visit
  \url{https://schalkdaniel.github.io/compboost/cpp_man/html/classlogger_1_1_iteration_logger.html}.
}

\section{Fields}{

  This class doesn't contain public fields.
}

\section{Methods}{

\describe{
\item{\code{summarizeLogger()}}{Summarize the logger object.}
}
}

\examples{
# Define logger:
log.iters = LoggerIteration$new(FALSE, 100)

# Summarize logger:
log.iters$summarizeLogger()

}
