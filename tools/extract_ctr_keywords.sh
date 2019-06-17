#!/bin/bash -e
################################################################################
#
# \file      tools/extract_ctr_keywords.sh
# \brief     Generate documentation pages for control file keywords
# \copyright 2012-2015 J. Bakosi,
#            2016-2018 Los Alamos National Security, LLC.,
#            2019 Triad National Security, LLC.
#            All rights reserved. See the LICENSE file for details.
# \details   This script runs an executable and extracts the help for each of
# the executable's control file keyword, describing their documentation,
# generating a documentation page with this information.
#
# A single command line argument is required: the name of the executable.
################################################################################

export exe=$1

if ( ../Main/$exe -h | grep '\-C' > /dev/null ); then

  # generate summary of all control file keywords
  export screen_out=$(../Main/$exe -C | awk "/$exe Control File Keywords/,0" | grep "^[[:space:]]\|^-" | sed 's/^/        /')
  perl -i -p0e 's/(\@section $ENV{exe}_ctr_list List of all control file keywords).*(\@section $ENV{exe}_ctr_detail)/$1\n\n$ENV{screen_out}\n\n$2/s' pages/${exe}_ctr.dox

  # generate detailed description on all keywords
  export keywords=$(../Main/$exe -C | awk "/$exe Control File Keywords/,0" | grep "^[[:space:]]\|^-" | awk '{print $1}')
  echo "Extracting details of control file keywords unimplemented!"
  #export detail=$(for k in $keywords; do ../Main/$exe -H $k ++quiet | grep -v '^Quinoa>' | sed "s/$exe control file keyword/@subsection ${exe}_ctr_kw_$k Keyword/" | sed 's/Expected type:/_Expected type:_/' | sed 's/Lower bound:/_Lower bound:_/' | sed 's/Upper bound:/_Upper bound:_/' | sed 's/Expected valid choices:/_Expected valid choices:_/'; done)
  #echo $detail
#  perl -i -p0e 's/(\@section $ENV{exe}_cmd_detail Detailed description of command line parameters).*(\*\/)/$1\n$ENV{detail}\n\n$2/s' pages/${exe}_cmd.dox

fi
