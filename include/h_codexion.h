/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   h_codexion.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 23:00:46 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/07/14 17:43:49 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef H_CODEXION_H
# define H_CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>

# ifdef CONFIG_COLOR
#  define RESET          "\033[0m"
#  define TIMESTAMP_CLR  "\033[90m"
#  define ID_CLR         "\033[36m"
#  define DONGLE_CLR     "\033[33m"
#  define COMPILE_CLR    "\033[32m"
#  define DEBUG_CLR      "\033[34m"
#  define REFACTOR_CLR   "\033[35m"
#  define DEATH_CLR      "\033[31m"
# else
#  define RESET          ""
#  define TIMESTAMP_CLR  ""
#  define ID_CLR         ""
#  define DONGLE_CLR     ""
#  define COMPILE_CLR    ""
#  define DEBUG_CLR      ""
#  define REFACTOR_CLR   ""
#  define DEATH_CLR      ""
# endif

void	print_error(int n);

#endif