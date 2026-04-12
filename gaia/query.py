#!/usr/bin/python3

import sys

from astroquery.gaia import Gaia

if len(sys.argv) != 2:
    print(f"usage: {sys.argv[0]} <result>.csv")
    exit(1)

OUTPUT_PATH = sys.argv[1]

query = """
SELECT TOP 250000
    gs.source_id,
    gs.ra,
    gs.dec,
    gs.parallax,

    ap.lum_flame,
    ap.teff_gspphot

FROM
    gaiadr3.gaia_source AS gs

JOIN gaiadr3.astrophysical_parameters AS ap
    ON gs.source_id = ap.source_id

WHERE
    gs.parallax BETWEEN 0.5 AND 14.0
    AND ap.lum_flame IS NOT NULL
    AND ap.teff_gspphot IS NOT NULL

ORDER BY ap.lum_flame DESC
"""

job = Gaia.launch_job_async(query)

results = job.get_results()

if results:
    results.write(OUTPUT_PATH, format="csv", overwrite=True)

    print(f"{OUTPUT_PATH}: {len(results)} column(s) written")

