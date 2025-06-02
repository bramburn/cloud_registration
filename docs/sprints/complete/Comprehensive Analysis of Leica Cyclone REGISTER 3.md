<img src="https://r2cdn.perplexity.ai/pplx-full-logo-primary-dark%402x.png" class="logo" width="120"/>

# Comprehensive Analysis of Leica Cyclone REGISTER 360 and FARO SCENE: Feature Sets and Registration Workflows

This analysis provides a detailed examination of two leading point cloud registration software platforms: Leica Cyclone REGISTER 360 and FARO SCENE. Both platforms represent industry standards for 3D laser scanning data processing, offering sophisticated registration capabilities, automated workflows, and comprehensive export functionality. The comparison reveals distinct approaches to user interface design, registration methodologies, and data management, providing valuable insights for developing competitive registration software solutions.

## Leica Cyclone REGISTER 360 Feature Set

### Core Registration Capabilities

Leica Cyclone REGISTER 360 represents a comprehensive point cloud registration platform built from the ground up with modern architectural principles[^1_2]. The software provides **automated registration** capabilities that significantly reduce complexity while enhancing processing speed[^1_2]. The platform supports multiple registration methodologies including **cloud-to-cloud registration** for datasets without targets, **automatic black and white target detection**, and **hybrid registration** that combines various registration approaches[^1_14].

The software incorporates **SmartAlign technology** which aids automatic cloud registration by allowing users to specify search parameters, reducing computational overhead by limiting comparisons to user-defined links[^1_14]. The system requires a minimum of three matching targets between setups to form target links, but can utilize cloud-to-cloud connections when fewer than three matching targets are available[^1_14]. This flexibility ensures robust registration even in challenging scanning scenarios.

### Workflow Architecture and User Interface

The platform follows a **guided four-step workflow** structure that includes Import, Review and Optimize, Finalize, and Report phases[^1_14]. This structured approach ensures consistent processing while accommodating users of varying skill levels[^1_2]. The software features **multi-threaded batch routines** and **one-step import and processing** capabilities that significantly accelerate project completion times[^1_2].

Advanced visualization tools include **3D point cloud navigation** with support for multiple viewing modes including 2D map view, 360-degree panoramic view, and full 3D visualization[^1_18]. The platform provides **real-time quality control** features enabling on-site verification of registration accuracy through the companion Cyclone FIELD 360 mobile application[^1_17][^1_18].

### Integration and Compatibility

Cyclone REGISTER 360 demonstrates exceptional hardware integration, particularly with Leica scanning systems including the **RTC360, BLK360, and ScanStation P-Series** scanners[^1_17][^1_18]. The software supports **Visual Inertial System (VIS) technology** for automated targetless registration, leveraging built-in scanner capabilities for enhanced accuracy[^1_18][^1_20].

The platform includes comprehensive **project management capabilities** with support for multiple storage locations, project archiving using RAF format, and collaborative workflows through cloud integration[^1_14]. Users can manage projects across multiple drives and create separate folders for storage and archiving, optimizing data organization and accessibility[^1_14].

## FARO SCENE Feature Set

### Registration Methodologies

FARO SCENE offers two primary registration approaches: **Interactive Registration** and **Hybrid Registration**[^1_5]. Interactive Registration provides an intuitive graphical user interface that allows users to visualize scan linkages during the registration process and manually control connection establishment[^1_3][^1_5]. This approach enables precise control over registration parameters while maintaining workflow efficiency.

**Hybrid Registration** represents a significant advancement, allowing **cloud-to-cloud, target-based, and survey control methods** to be used individually, in combination, or in various hybrid configurations[^1_3][^1_5]. This flexibility accommodates diverse scanning scenarios and project requirements, ensuring optimal registration accuracy regardless of available reference data.

### Visualization and Analysis Tools

SCENE features **advanced 3D visualization capabilities** with support for stunning 3D renderings of real-world objects and environments[^1_3]. The software includes an **impressive virtual reality (VR) view** that enables users to experience and evaluate captured data in immersive VR environments[^1_3]. This capability significantly enhances spatial understanding and data interpretation.

The platform provides comprehensive **export functionality** supporting various formats for different applications[^1_3]. Users can create **visually impressive outputs** suitable for documentation, analysis, and presentation purposes across multiple industries including public safety, forensics, and engineering[^1_3].

### Hardware Compatibility and Performance

FARO SCENE is specifically designed to process scans from **FARO Focus Laser Scanners, Focus Swift Indoor Mobile Scanner, Freestyle 2 Handheld Scanner, and third-party laser scanners**[^1_3]. The software features **user-guided workflows** with extended language support ensuring minimal learning curves for new users[^1_3].

Performance optimization includes **automatic functions** and **optimized PC hardware utilization** providing market-leading productivity workflows[^1_3]. According to manufacturer specifications, the software makes surveying **three times more efficient** than traditional methods[^1_3].

## Detailed Registration Workflow: Leica Cyclone REGISTER 360

### Project Initialization and Data Import

The registration workflow begins with **project creation** through the Project Explorer window[^1_14]. Users initiate new projects by entering project names and configuring basic parameters including coordinate systems and project storage locations[^1_14]. The import phase supports multiple data sources including direct scanner connections, file imports, and cloud-based data transfers[^1_14].

**Data import options** include automatic detection of BLK360 devices, drag-and-drop file import, and structured data imports from various scanner formats[^1_14]. The software automatically detects scan metadata including timestamps, scanner positions, and image data when available[^1_4]. Users can configure import parameters including target detection settings, image processing options, and pre-registration preferences[^1_14].

### Registration Configuration and Execution

The registration process begins with **data source selection** where users choose between Auto Target for black and white target-based registration, Auto Cloud for targetless cloud-to-cloud registration, or manual configuration for complex scenarios[^1_14]. The software automatically analyzes scan overlap, identifies common features, and establishes preliminary registration parameters[^1_4].

**Visual alignment tools** enable users to manually adjust scan positions when automatic registration requires refinement[^1_18]. The system provides **split view functionality** allowing simultaneous visualization of multiple scans during alignment verification[^1_14]. Users can create manual links between scans, adjust registration parameters, and verify accuracy through integrated quality control tools.

### Optimization and Quality Control

Following initial registration, the **bundle optimization process** refines registration accuracy by minimizing global registration errors[^1_4]. The software calculates registration statistics, identifies potential issues, and provides comprehensive error reporting[^1_9]. Users can review registration networks, analyze link quality, and make adjustments to improve overall accuracy.

**Quality control features** include visual inspection tools, statistical analysis capabilities, and automated error detection[^1_18]. The system highlights problematic areas, provides optimization suggestions, and enables iterative refinement until desired accuracy levels are achieved[^1_14].

## Detailed Registration Workflow: FARO SCENE

### Project Setup and Scan Management

FARO SCENE registration workflows begin with **project creation** and scan import processes[^1_5]. The software supports direct import from FARO scanner formats as well as third-party data sources[^1_3]. Users configure project parameters including coordinate systems, measurement units, and quality settings during initial setup[^1_10].

**Interactive Registration workflow** guides users through connection establishment between individual scans[^1_5]. The system provides graphical visualization of scan relationships, enabling users to understand spatial relationships and manually control link creation when necessary[^1_5]. This approach ensures optimal registration accuracy while maintaining user control over the process.

### Registration Execution and Refinement

The registration process utilizes **automatic connection detection** combined with manual verification capabilities[^1_5]. Users can leverage **cloud-to-cloud registration** for overlapping scan areas, **target-based registration** when survey targets are available, or **hybrid approaches** combining multiple methodologies[^1_5].

**Registration refinement tools** enable users to adjust connection parameters, modify link weights, and optimize registration accuracy[^1_5]. The software provides real-time feedback during adjustment processes, allowing users to observe the impact of parameter changes on overall registration quality[^1_5].

### Validation and Export Preparation

Following registration completion, SCENE provides comprehensive **validation tools** including statistical analysis, visual verification, and accuracy reporting[^1_3]. Users can generate detailed registration reports, analyze error distributions, and verify compliance with project specifications[^1_3].

The system prepares registered data for export through **format optimization** and **quality assurance processes**[^1_6]. Users can configure export parameters, select appropriate file formats, and ensure data integrity before final delivery[^1_6].

## Export Workflows and Data Management

### Leica Cyclone REGISTER 360 Export Capabilities

Cyclone REGISTER 360 provides **comprehensive export functionality** supporting multiple industry-standard formats[^1_7]. The platform supports **E57 format exports** with full compatibility formatting options designed to maximize third-party software integration[^1_7]. Users can export **structured E57 files** maintaining scan positions and associated imagery, or create **unified point clouds** for specific applications[^1_7].

**RCP format support** enables direct integration with Autodesk applications including ReCap and AutoCAD[^1_7]. The software provides both **unified RCP exports** creating single consolidated point clouds and **setup-based RCP exports** maintaining individual scan structures with associated imagery[^1_7]. Additional export formats include **PTS and PTX** formats for specialized applications requiring specific data structures[^1_7].

The export process includes **automated quality control** ensuring data integrity and format compliance[^1_7]. Users can configure export parameters including point density, color mapping, coordinate system transformations, and metadata inclusion[^1_7].

### FARO SCENE Export Workflows

FARO SCENE offers sophisticated **E57 export capabilities** with specific support for **360-degree image integration**[^1_6]. The export process requires careful configuration to ensure panoramic images are properly embedded and accessible in exported files[^1_6]. Users must select **Full Color Resolution Panorama Image** options to maintain image data during export[^1_6].

**Format flexibility** enables exports to various point cloud formats while maintaining data quality and structure[^1_6]. The software supports **structured E57 exports** recommended for optimal platform compatibility, **individual scan exports** for specific applications, and **unified exports** when structure preservation is not required[^1_6].

Export workflows include **comprehensive quality assurance** with validation checks ensuring exported data meets specification requirements[^1_6]. The system provides progress monitoring, error detection, and completion verification throughout the export process[^1_6].

## Conclusion

Both Leica Cyclone REGISTER 360 and FARO SCENE represent sophisticated point cloud registration platforms with distinct approaches to workflow design and user interaction. Leica's solution emphasizes **automated processing** with guided workflows designed to minimize complexity while maximizing efficiency[^1_2][^1_14]. The platform's integration with companion mobile applications and cloud services creates a comprehensive field-to-office ecosystem particularly suited to modern scanning workflows[^1_17][^1_18].

FARO SCENE provides **advanced manual control capabilities** through Interactive Registration while supporting automated processing when appropriate[^1_3][^1_5]. The platform's **hybrid registration approach** and comprehensive visualization tools, including VR capabilities, offer significant advantages for complex projects requiring detailed analysis and presentation[^1_3][^1_5].

For MVP development targeting similar functionality, key considerations include implementing **flexible registration methodologies** supporting both automated and manual approaches, **comprehensive export capabilities** ensuring broad format compatibility, and **intuitive user interfaces** that accommodate varying skill levels while providing advanced control when necessary. The analysis reveals that successful registration software must balance automation with user control, providing efficient workflows while maintaining the flexibility to handle diverse scanning scenarios and project requirements.

<div style="text-align: center">⁂</div>

[^1_1]: https://arxiv.org/html/2502.04734v1

[^1_2]: https://globalsurvey.co.nz/shop/solutions/surveying-gis/laser-scanning/point-cloud-software/leica-cyclone-register-360/

[^1_3]: https://www.faro.com/en/Products/Software/SCENE-Software

[^1_4]: https://www.youtube.com/watch?v=_2HW0sv3GBg

[^1_5]: https://knowledge.faro.com/Software/FARO_SCENE/SCENE/Interactive_Registration_Workflow_in_SCENE

[^1_6]: https://webapp.atis.cloud/support/article/44

[^1_7]: https://rcdocs.leica-geosystems.com/cyclone-register-360/latest/third-party-file-export

[^1_8]: http://arxiv.org/pdf/2305.07103.pdf

[^1_9]: https://rcdocs.leica-geosystems.com/cyclone-register-360/latest/apply-control-workflow

[^1_10]: https://knowledge.faro.com/Software/FARO_SCENE/SCENE/Export_a_Project_.lsproj_File_in_SCENE

[^1_11]: http://arxiv.org/pdf/1910.08338.pdf

[^1_12]: https://www.youtube.com/watch?v=W2D9rMAvKG0

[^1_13]: https://pmc.ncbi.nlm.nih.gov/articles/PMC9741593/

[^1_14]: https://shop.leica-geosystems.com/sites/default/files/2019-04/leica_cyclone_register_360_quickstartguide_en.pdf

[^1_15]: https://pmc.ncbi.nlm.nih.gov/articles/PMC8706624/

[^1_16]: https://pmc.ncbi.nlm.nih.gov/articles/PMC9104861/

[^1_17]: https://leica-geosystems.com/en-gb/products/laser-scanners/software/leica-cyclone/leica-cyclone-field-360

[^1_18]: https://shop.leica-geosystems.com/sites/default/files/2024-10/Leica-Cyclone-FIELD-360-DS-0424-LR.pdf

[^1_19]: https://apps.apple.com/gb/app/leica-cyclone-field-360/id1376463007

[^1_20]: https://www.youtube.com/watch?v=zFW5D0JFfY8

[^1_21]: https://www.semanticscholar.org/paper/ae714968211063f302a03e2631be2ebd062e97a7

[^1_22]: https://www.semanticscholar.org/paper/e80a8828ee52c78db417c840fbf3414f6b50b084

[^1_23]: https://www.semanticscholar.org/paper/5bb90f9eb3fe383d71223031d13cd6ab1717ac7a

[^1_24]: https://www.semanticscholar.org/paper/84fdeb35640b525ef927ead7aec84c89223031ec

[^1_25]: https://www.semanticscholar.org/paper/8bd33dcc086c7718a4d302392f1387c7ec19ae83

[^1_26]: https://www.semanticscholar.org/paper/b30591ea5e5a045e672357a8876deac821422da5

[^1_27]: https://www.semanticscholar.org/paper/c62daac79893dcd13f108f201b1e437257e8451b

[^1_28]: https://pubmed.ncbi.nlm.nih.gov/24137948/

[^1_29]: https://www.semanticscholar.org/paper/af48eccab403c58059b93f48430f2f9e4dad9a6b

[^1_30]: https://pmc.ncbi.nlm.nih.gov/articles/PMC8272138/

[^1_31]: https://arxiv.org/html/2306.12992v2

[^1_32]: https://arxiv.org/pdf/2203.16756.pdf

[^1_33]: https://pmc.ncbi.nlm.nih.gov/articles/PMC10465169/

[^1_34]: https://leica-geosystems.com/en-gb/products/laser-scanners/software/leica-cyclone/leica-cyclone-field-360/boosting-productivity-in-the-field

[^1_35]: https://www.sccssurvey.co.uk/leica-cyclone-field-360.html

[^1_36]: https://g2survey.com/leica-cyclone-register-360-plus/

[^1_37]: https://measur.ca/products/faro-scene

[^1_38]: https://leica-geosystems.com/-/media/files/leicageosystems/products/datasheets/leica-cyclone-field-360-ds-0424-lr.ashx?sc_lang=it-it\&hash=566A238A3D93B8D717560031443B84FC

[^1_39]: https://leica-geosystems.com/en-gb/products/laser-scanners/software/leica-cyclone/leica-cyclone-field-360/point-cloud-in-your-pocket

[^1_40]: https://apps.apple.com/us/app/leica-cyclone-field-360/id1376463007

[^1_41]: https://www.ncbi.nlm.nih.gov/pmc/articles/PMC6288436/

[^1_42]: https://www.ncbi.nlm.nih.gov/pmc/articles/PMC11756586/

[^1_43]: https://www.semanticscholar.org/paper/03093c8d293b9423938a4c843ea43f941228dd23

[^1_44]: https://www.semanticscholar.org/paper/03fb60afb87cd9fec33d790a0db074f541726811

[^1_45]: https://www.semanticscholar.org/paper/7e4c28d4a855a15cf17f13585bbaea564c6f5f9b

[^1_46]: https://www.semanticscholar.org/paper/86d0ade0352c6e05e68c2db66c55a8ee0910d98d

[^1_47]: https://www.ncbi.nlm.nih.gov/pmc/articles/PMC7940834/

[^1_48]: https://www.semanticscholar.org/paper/fcdbf54a4c177968c117929bccf691f3f1e4368f

[^1_49]: https://rcdocs.leica-geosystems.com/cyclone-register-360/2022.1.1/workflow-for-cyclone-field-360-and-blk360

[^1_50]: https://www.semanticscholar.org/paper/c668ba32c75003ac06991e5a2561490f42415ac4

[^1_51]: https://arxiv.org/abs/1805.04714

[^1_52]: https://www.ncbi.nlm.nih.gov/pmc/articles/PMC11207716/

[^1_53]: https://www.semanticscholar.org/paper/340e02f8006cae3faf0752e02bd99567bc06ba1c

[^1_54]: https://www.semanticscholar.org/paper/203be72eda6519aafc26246a6318dac68305964b

[^1_55]: https://www.semanticscholar.org/paper/93616f984fee92a39552e4d83569b86a5ed1be16

[^1_56]: https://www.semanticscholar.org/paper/c30d4ddef6d8bb1fad7a5f4384d11014e481dd00

[^1_57]: https://arxiv.org/abs/2209.02700

[^1_58]: https://arxiv.org/abs/2401.09252

[^1_59]: https://www.semanticscholar.org/paper/be94bd5fbdfef154e4e58542bd713b17863ff7d4

[^1_60]: https://knowledge.faro.com/Software/FARO_SCENE/SCENE

[^1_61]: https://www.faro.com/en/Resource-Library/Brochure/FARO-SCENE-Software

[^1_62]: https://www.faro.com/en/Resource-Library/Video/scene-2019-whats-new

[^1_63]: https://connect.hexagongeosystems.com/webinar

[^1_64]: https://webapp.atis.cloud/support/article/49

[^1_65]: https://www.youtube.com/watch?v=s2MAwRBISwA

[^1_66]: https://knowledge.faro.com/Software/FARO_SCENE/SCENE/Feature_Comparison_by_Version_for_SCENE

[^1_67]: https://www.faro.com/en/Resource-Library/Tech-Sheet/techsheet-faro-scene

[^1_68]: https://media.faro.com/-/media/Project/FARO/FARO/FARO/Resources/2021/01/15/22/34/Tech-Sheet-FARO-SCENE-ENG.pdf?rev=64f619001fe14b7e9c18d3773a178494

[^1_69]: https://shop.laserscanning-europe.com/FARO-Scene_2

[^1_70]: https://www.faro.com/en/Resource-Library/Video/Scanning-with-Focus-Premium-Max-and-FARO-SCENE-Integration

[^1_71]: https://sites.saic.edu/aoc/wp-content/uploads/sites/68/2019/10/faro_scene_finished.pdf


---

# Can you explain the step-by-step workflow for registration in Leica Cyclone REGISTER 360

<div style="text-align: center">⁂</div>

[^2_1]: https://www.youtube.com/watch?v=_2HW0sv3GBg

[^2_2]: https://www.scribd.com/document/498176471/Cyclone-REGISTER-360-User-Guide

[^2_3]: https://shop.leica-geosystems.com/sites/default/files/2019-04/leica_cyclone_register_360_1.5_quick_start_guide.pdf

[^2_4]: https://www.youtube.com/watch?v=s2MAwRBISwA

[^2_5]: http://www.imaginit.com/portals/4/documents/Leica Cyclone REGISTER 360 FLY 865476 1117 enus LR.pdf_c736041a1O.pdf

[^2_6]: https://adp.uq.edu.au/leica-360-software

[^2_7]: https://www.youtube.com/watch?v=V3jGYOJXyT4

[^2_8]: https://www.youtube.com/watch?v=W2D9rMAvKG0

[^2_9]: https://rcdocs.leica-geosystems.com/cyclone-register-360/latest/apply-control-workflow

[^2_10]: https://resources.imaginit.com/reality-capture/creating-a-ucs-in-leica-cyclone-register-360

[^2_11]: https://www.youtube.com/watch?v=KMI5aW0K9VE

[^2_12]: https://rcdocs.leica-geosystems.com/cyclone-register-360/latest/report-publish


---

## Key Features of Leica Cyclone REGISTER 360

- **Simple Guided Workflow**: Step-by-step process—Import, Review and Optimize, Finalize, and Publish—designed for ease of use and minimal learning curve[^3_1][^3_2][^3_7][^3_11].
- **Automated Registration**: Automatic alignment of scans, supporting both target-based and targetless (cloud-to-cloud) registration, leveraging technologies like RTC360’s Visual Inertial System (VIS) for automated placement on real-world coordinates[^3_2][^3_7][^3_11].
- **Visual QA/QC Tools**: Built-in visual quality assurance tools let users easily assess the alignment and quality of registrations, with intuitive reporting to quickly identify and resolve issues[^3_1][^3_7][^3_11].
- **Multi-threaded Batch Processing**: High-speed, one-step import and processing of large projects (up to 500 scans), with batch routines to accelerate workflows[^3_1][^3_7][^3_11].
- **Comprehensive Sensor Support**: Compatible with all Leica Geosystems HDS sensors and third-party/industry-standard data formats[^3_1][^3_6].
- **Data Cleaning and Preparation**: Includes automated and manual tools for cleaning point clouds, such as moving object filters, surface smoothing, and anonymization features for privacy[^3_6].
- **Integrated Field-to-Office Workflow**: Seamless integration with Cyclone FIELD 360 for real-time field data streaming, tagging, and collaboration between field and office teams[^3_6].
- **SiteMap Feature**: Overlay scan positions on real-world or imported images for spatial context and reporting[^3_7].
- **Flexible Export Options**: Export to Leica-native formats (LGSx), Autodesk ReCap (RCP), and industry-standard formats (E57, PTX, PTS, etc.), supporting downstream use in design and analysis software[^3_1][^3_2][^3_7].
- **Scalability and Simplicity**: Designed to handle both small and large, complex projects with a user-friendly interface that supports users of any skill level[^3_2][^3_11].
- **Automated Reporting**: Generates detailed registration and quality reports for project documentation and client delivery[^3_1][^3_7].

---

## Key Features of FARO SCENE

- **Interactive Registration**: User-friendly graphical interface for visualizing and manually controlling scan linkages, providing transparency and control over the registration process[^3_4][^3_9][^3_12][^3_15].
- **Hybrid Registration**: Combines cloud-to-cloud, target-based, and survey control registration methods, allowing them to be used individually or in combination for maximum flexibility and accuracy[^3_4][^3_9][^3_12][^3_15].
- **Automatic Object Recognition**: Detects artificial (spheres, checkerboards, coded markers) and natural references (corners, planes) for automated registration and positioning[^3_8][^3_14].
- **Real-Time, On-Site Registration**: Enables immediate processing and registration of scan data on-site, allowing users to verify results and retake photos if needed[^3_5][^3_13].
- **Advanced Visualization**: Offers immersive 2D, 3D, and Virtual Reality (VR) views for exploring and evaluating point clouds, including solid surface rendering and HDR colorization[^3_5][^3_13][^3_14].
- **Data Cleaning and Filtering**: Automated filters for moving objects, stray points, dark points, and edge artifacts, reducing manual cleaning effort and improving data quality[^3_5][^3_8].
- **Batch Processing**: Supports automated marker detection, scan optimization, and registration for efficient handling of large datasets[^3_5][^3_8].
- **Intuitive Data Organization**: Hierarchical data structures, project history management, and easy-to-learn interface for efficient project handling[^3_5].
- **Collaboration and Sharing**: Seamless integration with SCENE WebShare Cloud for global, secure sharing and collaboration; SCENE 2go App for portable project sharing[^3_5][^3_13].
- **Comprehensive Export Options**: Supports export to a wide range of point cloud and CAD formats (E57, PTX, PTS, RCP, meshes, orthophotos, panoramic images, etc.)[^3_5][^3_8][^3_13].
- **Plug-in and App Support**: Extendable via plug-ins and apps for special workflows and applications[^3_8][^3_14].
- **Survey Control Integration**: Incorporate survey reference points for enhanced accuracy in cloud-to-cloud projects[^3_13][^3_14].
- **On-Site Compensation**: Verification and adjustment of scanner calibration available for supported devices[^3_8].

---

## Summary Table

| Feature Category | Leica Cyclone REGISTER 360 | FARO SCENE |
| :-- | :-- | :-- |
| Registration Methods | Target-based, Cloud-to-Cloud, Hybrid | Interactive, Hybrid (Cloud, Targets, Survey) |
| Guided Workflow | Yes (step-by-step) | Yes (user-guided, visual feedback) |
| Automation | High (auto-registration, batch) | High (auto registration, batch, filters) |
| QA/QC Tools | Visual, automated reports | Visual, workflow-driven validation |
| Visualization | 2D/3D, SiteMap overlay | 2D/3D, VR, HDR, immersive |
| Data Cleaning | Filters, anonymization, smoothing | Filters, moving object removal, edge artifact |
| Collaboration | Field-to-office, cloud, tagging | WebShare Cloud, SCENE 2go, global sharing |
| Export Formats | LGSx, RCP, E57, PTX, PTS, etc. | E57, PTX, PTS, RCP, meshes, images, etc. |
| Hardware Support | Leica HDS, BLK, third-party | FARO Focus, Swift, Freestyle, third-party |
| Plug-in/App Support | No | Yes |
| On-Site Registration | Yes (with FIELD 360) | Yes |


---

Both solutions are designed for speed, scalability, and simplicity, with robust automation, advanced visualization, and strong support for collaboration and export, but FARO SCENE stands out for its VR capabilities, plug-in extensibility, and highly interactive registration interface, while Leica Cyclone REGISTER 360 excels in field-to-office integration and guided, automated workflows[^3_1][^3_2][^3_4][^3_5][^3_6][^3_7][^3_8][^3_9][^3_11][^3_12][^3_13][^3_14][^3_15].

<div style="text-align: center">⁂</div>

[^3_1]: https://shop.leica-geosystems.com/gb/leica-blk/software/leica-cyclone-register-360-plus/buy

[^3_2]: https://globalsurvey.co.nz/shop/solutions/surveying-gis/laser-scanning/point-cloud-software/leica-cyclone-register-360/

[^3_3]: https://arxiv.org/abs/2209.12962

[^3_4]: https://www.faro.com/en/Resource-Library/Video/Scanning-with-Focus-Premium-Max-and-FARO-SCENE-Integration

[^3_5]: https://www.geosense.gr/wp-content/uploads/2022/01/Faro-Scene-Software-Datasheet.pdf

[^3_6]: https://g2survey.com/leica-cyclone-register-360-plus/

[^3_7]: https://shop.leica-geosystems.com/sites/default/files/2019-04/leica_cyclone_register_360_1.5_data_sheet.pdf

[^3_8]: https://media.faro.com/-/media/Project/FARO/FARO/FARO/Resources/2021/01/15/22/34/Tech-Sheet-FARO-SCENE-ENG.pdf?rev=64f619001fe14b7e9c18d3773a178494

[^3_9]: https://www.faro.com/en/Products/Software/SCENE-Software

[^3_10]: https://www.youtube.com/watch?v=ryTKCbP9_iw

[^3_11]: https://www.sccssurvey.co.uk/leica-cyclone-register-360.html

[^3_12]: https://groupsynergy.com/faro-scene-software

[^3_13]: https://www.sunbeltsales.co.uk/faro-scene

[^3_14]: https://www.faro.com/en/Resource-Library/Tech-Sheet/techsheet-faro-scene

[^3_15]: https://www.dicarlotech.com/products/software/laser-scanning/faro-scene

[^3_16]: https://pubmed.ncbi.nlm.nih.gov/39598976/

[^3_17]: https://www.semanticscholar.org/paper/87bdbf45a3c4c0de6449522c4d7060cf3c4f2faf

[^3_18]: https://www.semanticscholar.org/paper/ac0a2342ce59baa437ac6ec926f1feb886d23a4e

[^3_19]: https://www.semanticscholar.org/paper/2018567ac0bbe0ec15630c0cf1879909f4b1c26f

[^3_20]: https://www.ncbi.nlm.nih.gov/pmc/articles/PMC7769690/

[^3_21]: https://www.semanticscholar.org/paper/ea01d2a531419a3ed5d45932080ce174d85b293b

[^3_22]: https://www.semanticscholar.org/paper/b7f4b596b44441d98819be3d4a83536c077d2f5f

[^3_23]: https://arxiv.org/abs/2101.07314

[^3_24]: https://www.ncbi.nlm.nih.gov/pmc/articles/PMC6015267/

[^3_25]: https://www.ncbi.nlm.nih.gov/pmc/articles/PMC10971669/

[^3_26]: https://leica-geosystems.com/en-gb/products/laser-scanners/software/leica-cyclone/leica-cyclone-register-360

[^3_27]: https://www.youtube.com/watch?v=_2HW0sv3GBg

[^3_28]: https://lordtechsurvey.com/product/leica-cyclone-register360-software/

[^3_29]: https://arxiv.org/abs/2110.07703

[^3_30]: https://arxiv.org/abs/2204.01587

[^3_31]: https://www.semanticscholar.org/paper/e1bb5e4a5438ab3eb0a017633c13d11dafafbd9d

[^3_32]: https://www.semanticscholar.org/paper/cb56c50494090f7442e621f78d04c51deb7a9ddc

[^3_33]: https://www.semanticscholar.org/paper/35e6485bb304115404ddc84263059b38dfe065e3

[^3_34]: https://arxiv.org/abs/2302.12251

[^3_35]: https://www.semanticscholar.org/paper/a8984bb4a4374ea82494f23ddcce76a121a72ccf

[^3_36]: https://www.semanticscholar.org/paper/5e20c54f789a990ccec7d710b9c26e7a7adfb3dc

[^3_37]: https://arxiv.org/abs/2404.01223

[^3_38]: https://arxiv.org/abs/2403.15624

[^3_39]: https://knowledge.faro.com/Software/FARO_SCENE/SCENE

[^3_40]: https://knowledge.faro.com/Software/FARO_SCENE/SCENE/Feature_Comparison_by_Version_for_SCENE

[^3_41]: https://knowledge.faro.com/Software/FARO_SCENE/SCENE/Interactive_Registration_Workflow_in_SCENE

[^3_42]: https://www.faro.com/en/News-Library/2022/FARO-Zone-3D-Forensic-Scene-Analysis-Software-Delivers-New-Photogrammetry-Capabilities

[^3_43]: https://www.youtube.com/watch?v=1ZbDkcsS-xk

[^3_44]: https://datumtechsolutions.com/products/leica-register-360-plus-software

[^3_45]: https://pmc.ncbi.nlm.nih.gov/articles/PMC3127962/

[^3_46]: https://arxiv.org/pdf/2206.15447.pdf


---

# How does FARO SCENE handle scan registration from data import to export

FARO SCENE's registration workflow integrates automated processing with manual refinement tools, ensuring efficient data alignment from import to export. Here's the detailed workflow:

---

## **1. Data Import \& Project Setup**

- **Supported Formats**: Direct import from FARO Focus, Freestyle, Swift scanners, and third-party data (E57, PTX, PTS) [^4_4][^4_5].
- **Automatic Transfer**: Inserting a scanner’s SD card triggers automatic project import, preserving metadata (coordinates, timestamps) [^4_2][^4_4].
- **Project Structure**:
    - Hierarchical organization of scans into clusters.
    - Automatic creation of preview thumbnails and point clouds for visualization [^4_2][^4_5].

---

## **2. Registration Workflow**

### **A. Preprocessing \& Automatic Registration**

1. **Preprocess Scans**:
    - Enable **Place Scans** to configure registration method:
        - **Target-Based**: Detects spheres, checkerboards, or coded markers.
        - **Cloud-to-Cloud**: Matches overlapping geometry in scan pairs.
        - **Hybrid**: Combines targets and cloud-to-cloud for complex projects [^4_4][^4_5].
    - Batch-process scans to auto-detect common references [^4_2][^4_4].
2. **Registration Execution**:
    - Automatically aligns scans using selected method.
    - Generates a registration network showing scan connections [^4_2][^4_5].

### **B. Interactive Refinement**

- **Visual Feedback**: 3D/2D views highlight misalignments with color-coded error indicators [^4_4][^4_5].
- **Manual Adjustments**:
    - Adjust scan positions via drag-and-drop in the 3D view.
    - Modify link weights or delete unreliable connections [^4_2][^4_5].
- **Survey Control Integration**: Add ground control points (GCPs) to improve absolute accuracy [^4_5].

---

## **3. Validation \& Error Correction**

- **Quality Metrics**:
    - **Registration Report**: Lists residual errors per scan connection [^4_5].
    - **Statistical Analysis**: RMS error visualization for global/local adjustments [^4_4].
- **Moving Object Filter**: Automatically removes transient objects (e.g., people, vehicles) [^4_4][^4_5].
- **Bundle Adjustment**: Optimizes all scans simultaneously to minimize global error [^4_2].

---

## **4. Data Optimization for Export**

- **Filtering Tools**:
    - Remove stray points, edge artifacts, and noise [^4_5].
    - Apply color balancing for consistent HDR imagery [^4_4].
- **Scan Point Cloud Generation**:
    - Required for export; converts raw data to processed point clouds [^4_2].
- **Clipping Boxes**: Define regions of interest to reduce export size [^4_2].

---

## **5. Export Workflow**

- **Format Options**:
    - **Point Clouds**: E57 (with HDR images), RCP (Autodesk), PTX, PTS [^4_4][^4_6].
    - **Meshes/Orthophotos**: For CAD/BIM integration [^4_5].
- **Export Settings**:
    - **Ordered vs. Unordered**: Preserve scan structure or merge into single cloud [^4_6].
    - **Coordinate System**: Apply transformations (e.g., local to global CRS) [^4_5][^4_7].
- **FLS File Handling**:
    - Use *"Save Copy of Scan"* to embed registration data into FLS files [^4_1].
    - Export as new project to retain all revisions [^4_1][^4_8].

---

## **Key Differentiators**

| Feature | FARO SCENE Advantage |
| :-- | :-- |
| **On-Site Registration** | Real-time processing during scanning; retake photos if gaps detected [^4_4][^4_5]. |
| **VR Visualization** | Immersive inspection in virtual reality mode [^4_4][^4_5]. |
| **Hybrid Registration** | Combine targets, cloud-to-cloud, and survey control in one workflow [^4_4][^4_5]. |
| **WebShare Integration** | Publish projects directly to cloud for client collaboration [^4_4][^4_5]. |

For MVP development, prioritize implementing **interactive registration controls**, **hybrid alignment methods**, and **E57/RCP export with HDR support** to match FARO SCENE’s core functionality.

<div style="text-align: center">⁂</div>

[^4_1]: https://knowledge.faro.com/Software/FARO_SCENE/SCENE/Exporting_FLS_Files_with_Registration_Information_in_SCENE

[^4_2]: https://sites.saic.edu/aoc/wp-content/uploads/sites/68/2019/10/faro_scene_finished.pdf

[^4_3]: https://www.youtube.com/watch?v=y2dmm3REeP8

[^4_4]: https://media.faro.com/-/media/Project/FARO/FARO/FARO/Resources/2_TECH-SHEET/SCENE-TechSheets/TechSheet_SCENE_Software_EN.pdf?rev=abb303eba7f14737badb20e201e64d7b

[^4_5]: https://media.faro.com/-/media/Project/FARO/FARO/FARO/Resources/2021/01/15/22/34/Tech-Sheet-FARO-SCENE-ENG.pdf?rev=64f619001fe14b7e9c18d3773a178494

[^4_6]: https://knowledge.faro.com/Software/FARO_SCENE/SCENE/Export_Scan_Settings_in_SCENE

[^4_7]: https://www.youtube.com/watch?v=WbUt0Yu4Rh8

[^4_8]: https://www.youtube.com/watch?v=I86r7oggMC0

[^4_9]: https://www.semanticscholar.org/paper/e80e7c4edd01def0c5b5a9b1cbfaf827ec084276

